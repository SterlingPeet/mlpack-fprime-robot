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

// The write buffer covers the full 28-byte PololuRPiSlave struct plus one
// leading register-address byte.  Writing zeros to bytes 1-21 (the sensor-
// owned fields) is safe: the 32U4 loop unconditionally overwrites them with
// fresh readings before every finalizeWrites().
static constexpr FwSizeType WRITE_BUF_LEN = 29;  // 1 reg-addr + 28 struct bytes
static constexpr FwSizeType TELEM_BUF_LEN = 28;

// Byte offsets within m_writeBuf for the Pi-writable fields.
// Formula: struct_offset + 1  (the +1 accounts for the leading reg-addr byte).
static constexpr FwSizeType WOFF_YELLOW = 22;   // struct offset 21
static constexpr FwSizeType WOFF_GREEN = 23;    // struct offset 22
static constexpr FwSizeType WOFF_RED = 24;      // struct offset 23
static constexpr FwSizeType WOFF_LMOT_LO = 25;  // struct offset 24 (leftMotor lo)
static constexpr FwSizeType WOFF_LMOT_HI = 26;  // struct offset 25 (leftMotor hi)
static constexpr FwSizeType WOFF_RMOT_LO = 27;  // struct offset 26 (rightMotor lo)
static constexpr FwSizeType WOFF_RMOT_HI = 28;  // struct offset 27 (rightMotor hi)

// Byte offsets in the received telemetry buffer (struct offsets, 0-based).
static constexpr FwSizeType OFF_LEFT_ENC = 0;
static constexpr FwSizeType OFF_RIGHT_ENC = 2;
static constexpr FwSizeType OFF_BATT_MV = 4;
static constexpr FwSizeType OFF_BUTTON_A = 18;
static constexpr FwSizeType OFF_BUTTON_B = 19;
static constexpr FwSizeType OFF_BUTTON_C = 20;

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

RomiHWDriver::RomiHWDriver(const char* const compName) : RomiHWDriverComponentBase(compName) {
    // Write buffer: byte 0 = register address 0; bytes 1-21 = zero padding
    // (sensor-owned fields); bytes 22-28 = Pi-owned output fields.
    m_writeBuf[0] = 0;
    std::memset(m_writeBuf + 1, 0, WRITE_BUF_LEN - 1);
    m_writeFwBuf = Fw::Buffer(m_writeBuf, WRITE_BUF_LEN);

    // Telemetry receive buffer.
    std::memset(m_telemBuf, 0, TELEM_BUF_LEN);
    m_telemFwBuf = Fw::Buffer(m_telemBuf, TELEM_BUF_LEN);

    // Cached output state.
    std::memset(m_ledCache, 0, sizeof(m_ledCache));
    m_motorCache[0] = 0;
    m_motorCache[1] = 0;

    // Initialise to 0 so the first read triggers change notifications.
    std::memset(m_lastButtonState, 0, sizeof(m_lastButtonState));
}

RomiHWDriver::~RomiHWDriver() {}

// -------------------------------------------------------------------------
// Private helper: single I2C write+read cycle
// -------------------------------------------------------------------------

bool RomiHWDriver::performI2cCycle(U8 i2cAddr) {
    // Pack the cached output state into the write buffer at the Pi-writable
    // offsets.  Bytes 1-21 remain zero; the 32U4 overwrites those fields with
    // fresh sensor readings before finalizeWrites(), so they are never stale.
    m_writeBuf[WOFF_YELLOW] = m_ledCache[0];
    m_writeBuf[WOFF_GREEN] = m_ledCache[1];
    m_writeBuf[WOFF_RED] = m_ledCache[2];
    m_writeBuf[WOFF_LMOT_LO] = static_cast<U8>(m_motorCache[0] & 0xFF);
    m_writeBuf[WOFF_LMOT_HI] = static_cast<U8>((m_motorCache[0] >> 8) & 0xFF);
    m_writeBuf[WOFF_RMOT_LO] = static_cast<U8>(m_motorCache[1] & 0xFF);
    m_writeBuf[WOFF_RMOT_HI] = static_cast<U8>((m_motorCache[1] >> 8) & 0xFF);

    // Single transaction: write 29 bytes starting at reg 0, sleep 100 µs,
    // then read back the 28-byte telemetry struct.
    const Drv::I2cStatus s = this->i2cWriteRead_out(0, i2cAddr, m_writeFwBuf, m_telemFwBuf);
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

    // Emit telemetry channels.
    this->tlmWrite_BatteryVoltage(battV);
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

}  // namespace ROMI
