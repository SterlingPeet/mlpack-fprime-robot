// ======================================================================
// \title  RomiHWDriver.cpp
// \author Sterling Peet <sterling.peet@gatech.edu>
// \brief  Hardware abstraction layer for the Romi 32U4 over I2C
// ======================================================================

#include "Components/RomiHWDriver/RomiHWDriver.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"
#include "Fw/Types/BasicTypes.hpp"

#include <cstring>

namespace ROMI {

// -------------------------------------------------------------------------
// Wire-format constants
// -------------------------------------------------------------------------

// Register addresses in the Romi firmware register map (byte offsets).
static constexpr U8 REG_TELEM = 0;       // first telemetry register (read)
static constexpr U8 REG_LED_MOTOR = 21;  // first writable command register
static constexpr U8 REG_NOTES = 28;      // playNotes flag, then notes[14]

// Sizes of the static I/O buffers.
static constexpr FwSizeType CMD_BUF_LEN = 8;     // reg + 3 LED + 4 motor bytes
static constexpr FwSizeType NOTES_BUF_LEN = 16;  // reg + playNotes + 14 notes
static constexpr FwSizeType TELEM_BUF_LEN = 21;  // registers 0..20

// Byte offsets within m_cmdBuf.
static constexpr FwSizeType COFF_REG = 0;       // = REG_LED_MOTOR
static constexpr FwSizeType COFF_YELLOW = 1;    // reg 21
static constexpr FwSizeType COFF_GREEN = 2;     // reg 22
static constexpr FwSizeType COFF_RED = 3;       // reg 23
static constexpr FwSizeType COFF_LMOT_LO = 4;   // reg 24 (leftMotor lo)
static constexpr FwSizeType COFF_LMOT_HI = 5;   // reg 25 (leftMotor hi)
static constexpr FwSizeType COFF_RMOT_LO = 6;   // reg 26 (rightMotor lo)
static constexpr FwSizeType COFF_RMOT_HI = 7;   // reg 27 (rightMotor hi)

// Byte offsets in the received telemetry buffer (register == offset).
static constexpr FwSizeType OFF_LEFT_ENC = 0;
static constexpr FwSizeType OFF_RIGHT_ENC = 2;
static constexpr FwSizeType OFF_BATT_MV = 4;
static constexpr FwSizeType OFF_ANALOG = 6;  // 6 x U16 -> offsets 6,8,10,12,14,16
static constexpr FwSizeType OFF_BUTTON_A = 18;
static constexpr FwSizeType OFF_BUTTON_B = 19;
static constexpr FwSizeType OFF_BUTTON_C = 20;

static constexpr FwSizeType ANALOG_COUNT = 6;

// -------------------------------------------------------------------------
// Little-endian read helpers
// -------------------------------------------------------------------------

static I16 readI16LE(const U8* buf, FwSizeType off) {
    return static_cast<I16>(static_cast<U16>(buf[off]) | (static_cast<U16>(buf[off + 1]) << 8));
}

static U16 readU16LE(const U8* buf, FwSizeType off) {
    return static_cast<U16>(buf[off]) | (static_cast<U16>(buf[off + 1]) << 8);
}

// -------------------------------------------------------------------------
// Construction and destruction
// -------------------------------------------------------------------------

RomiHWDriver::RomiHWDriver(const char* const compName)
    : RomiHWDriverComponentBase(compName), m_playNotesPending(false) {
    // Command write buffer: byte 0 = register 21, bytes 1-7 = LED + motor state.
    std::memset(m_cmdBuf, 0, CMD_BUF_LEN);
    m_cmdBuf[COFF_REG] = REG_LED_MOTOR;
    m_cmdFwBuf = Fw::Buffer(m_cmdBuf, CMD_BUF_LEN);

    // Notes write buffer: byte 0 = register 28, byte 1 = playNotes flag.
    std::memset(m_notesBuf, 0, NOTES_BUF_LEN);
    m_notesBuf[0] = REG_NOTES;
    m_notesBuf[1] = 1;
    m_notesFwBuf = Fw::Buffer(m_notesBuf, NOTES_BUF_LEN);

    // Register-address buffer for telemetry reads (always register 0).
    m_regBuf[0] = REG_TELEM;
    m_regFwBuf = Fw::Buffer(m_regBuf, sizeof(m_regBuf));

    // Telemetry receive buffer.
    std::memset(m_telemBuf, 0, TELEM_BUF_LEN);
    m_telemFwBuf = Fw::Buffer(m_telemBuf, TELEM_BUF_LEN);

    // Cached output state.
    std::memset(m_ledCache, 0, sizeof(m_ledCache));
    m_motorCache[0] = 0;
    m_motorCache[1] = 0;
    std::memset(m_notes, 0, sizeof(m_notes));

    // Initialise to 0 so the first read triggers change notifications.
    std::memset(m_lastButtonState, 0, sizeof(m_lastButtonState));
}

RomiHWDriver::~RomiHWDriver() {}

// -------------------------------------------------------------------------
// Private helper: flush command writes, then read telemetry
// -------------------------------------------------------------------------

bool RomiHWDriver::performI2cCycle(U8 i2cAddr) {
    // 1. Pack and flush the LED + motor command block (write to register 21).
    m_cmdBuf[COFF_YELLOW] = m_ledCache[0];
    m_cmdBuf[COFF_GREEN] = m_ledCache[1];
    m_cmdBuf[COFF_RED] = m_ledCache[2];
    m_cmdBuf[COFF_LMOT_LO] = static_cast<U8>(m_motorCache[0] & 0xFF);
    m_cmdBuf[COFF_LMOT_HI] = static_cast<U8>((m_motorCache[0] >> 8) & 0xFF);
    m_cmdBuf[COFF_RMOT_LO] = static_cast<U8>(m_motorCache[1] & 0xFF);
    m_cmdBuf[COFF_RMOT_HI] = static_cast<U8>((m_motorCache[1] >> 8) & 0xFF);

    if (this->i2cWrite_out(0, i2cAddr, m_cmdFwBuf) != Drv::I2cStatus::I2C_OK) {
        return false;
    }

    // 2. Flush a pending note sequence as a separate write to register 28.
    if (m_playNotesPending) {
        std::memcpy(m_notesBuf + 2, m_notes, sizeof(m_notes));
        const Drv::I2cStatus ns = this->i2cWrite_out(0, i2cAddr, m_notesFwBuf);
        if (ns == Drv::I2cStatus::I2C_OK) {
            m_playNotesPending = false;
        }
        // A failed notes write leaves the request pending for the next cycle.
    }

    // 3. Read the telemetry block: write register address 0, delay, read 21
    //    bytes.  The delay and the discrete write/read live in RomiI2cDriver.
    const Drv::I2cStatus s = this->i2cWriteRead_out(0, i2cAddr, m_regFwBuf, m_telemFwBuf);
    return (s == Drv::I2cStatus::I2C_OK);
}

// -------------------------------------------------------------------------
// schedIn_handler
// -------------------------------------------------------------------------

void RomiHWDriver::schedIn_handler(FwIndexType portNum, U32 context) {
    Fw::ParamValid valid;
    const U8 i2cAddr = this->paramGet_I2C_ADDRESS(valid);

    if (!this->performI2cCycle(i2cAddr)) {
        return;
    }

    // Parse encoders.
    const I16 leftEnc = readI16LE(m_telemBuf, OFF_LEFT_ENC);
    const I16 rightEnc = readI16LE(m_telemBuf, OFF_RIGHT_ENC);

    // Parse battery voltage: millivolts -> volts.
    const U16 battMv = readU16LE(m_telemBuf, OFF_BATT_MV);
    const F32 battV = static_cast<F32>(battMv) / 1000.0f;

    // Parse analog channels.
    ROMI::RomiAnalog analog;
    for (FwSizeType i = 0; i < ANALOG_COUNT; ++i) {
        analog[i] = readU16LE(m_telemBuf, OFF_ANALOG + (2 * i));
    }

    // Emit telemetry channels.
    this->tlmWrite_BatteryVoltage(battV);
    this->tlmWrite_AnalogChannels(analog);
    this->tlmWrite_i2cAddress(i2cAddr);

    // Push encoder values if the downstream port is wired.
    if (this->isConnected_sendEncoders_OutputPort(0)) {
        ROMI::MotorEncoders encoders(leftEnc, rightEnc);
        this->sendEncoders_out(0, encoders);
    }

    // Button change detection for A, B, C.
    static const ROMI::RomiButton BUTTON_ENUM[3] = {
        ROMI::RomiButton::A,
        ROMI::RomiButton::B,
        ROMI::RomiButton::C,
    };
    const FwSizeType buttonOffsets[3] = {OFF_BUTTON_A, OFF_BUTTON_B, OFF_BUTTON_C};

    for (FwIndexType i = 0; i < 3; ++i) {
        const U8 newState = (m_telemBuf[buttonOffsets[static_cast<FwSizeType>(i)]] != 0) ? 1 : 0;
        if (newState != m_lastButtonState[i]) {
            m_lastButtonState[i] = newState;
            const ROMI::ButtonState bState = (newState != 0) ? ROMI::ButtonState::PRESSED : ROMI::ButtonState::RELEASED;
            if (this->isConnected_sendButton_OutputPort(i)) {
                Fw::Logic logicOut = (newState != 0) ? Fw::Logic::HIGH : Fw::Logic::LOW;
                (void)this->sendButton_out(i, logicOut);
            } else {
                this->log_ACTIVITY_HI_ButtonStateChange(BUTTON_ENUM[i], bState);
            }
        }
    }
}

// -------------------------------------------------------------------------
// setLed_handler
// -------------------------------------------------------------------------

Drv::GpioStatus RomiHWDriver::setLed_handler(FwIndexType portNum, const Fw::Logic& state) {
    if (portNum < 0 || portNum >= 3) {
        return Drv::GpioStatus::INVALID_MODE;
    }
    m_ledCache[portNum] = (state == Fw::Logic::HIGH) ? 1 : 0;
    return Drv::GpioStatus::OP_OK;
}

// -------------------------------------------------------------------------
// setMotors_handler
// -------------------------------------------------------------------------

void RomiHWDriver::setMotors_handler(FwIndexType portNum, const ROMI::MotorCommand& speed) {
    m_motorCache[0] = speed.get_left();
    m_motorCache[1] = speed.get_right();
}

// -------------------------------------------------------------------------
// LED command handlers
// -------------------------------------------------------------------------

void RomiHWDriver::SET_YELLOW_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) {
    m_ledCache[0] = (state == Fw::On::ON) ? 1 : 0;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RomiHWDriver::SET_GREEN_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) {
    m_ledCache[1] = (state == Fw::On::ON) ? 1 : 0;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RomiHWDriver::SET_RED_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) {
    m_ledCache[2] = (state == Fw::On::ON) ? 1 : 0;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// -------------------------------------------------------------------------
// PLAY_NOTES command handler
// -------------------------------------------------------------------------

void RomiHWDriver::PLAY_NOTES_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdStringArg& notes) {
    // Copy up to 14 chars (NUL-padded) into the cached note buffer; the
    // write is batched into the next schedIn alongside other I2C traffic.
    std::memset(m_notes, 0, sizeof(m_notes));
    const char* src = notes.toChar();
    for (FwSizeType i = 0; i < sizeof(m_notes) && src[i] != '\0'; ++i) {
        m_notes[i] = src[i];
    }
    m_playNotesPending = true;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace ROMI
