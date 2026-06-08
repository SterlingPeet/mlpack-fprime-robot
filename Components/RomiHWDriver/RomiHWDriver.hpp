// ======================================================================
// \title  RomiHWDriver.hpp
// \brief  Hardware abstraction layer for the Romi 32U4 over I2C
//
// On every schedIn tick this component issues these I2C transactions
// against the register-addressed Romi firmware:
//   1. write:     [reg=21, yellow, green, red, L_lo, L_hi, R_lo, R_hi]
//   2. write:     [reg=28, playNotes=1, notes×14]      (only when pending)
//   3. writeRead: write [reg=0], delay, read 21 bytes  (telemetry block)
// Reads never use a repeated start: the driver writes the register address
// in one transaction and reads in the next (see RomiI2cDriver).  setMotors,
// setLed and PLAY_NOTES only cache values; hardware I/O is batched into
// schedIn to keep I2C traffic deterministic at 50 Hz.
// ======================================================================

#ifndef ROMI_RomiHWDriver_HPP
#define ROMI_RomiHWDriver_HPP

#include "Components/RomiHWDriver/RomiHWDriverComponentAc.hpp"
#include "Drv/Ports/GpioStatusEnumAc.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"
#include "Fw/Buffer/Buffer.hpp"
#include "Fw/Types/BasicTypes.hpp"

namespace ROMI {

class RomiHWDriver : public RomiHWDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    explicit RomiHWDriver(const char* const compName);
    ~RomiHWDriver();

  private:
    // ----------------------------------------------------------------------
    // Port handler implementations
    // ----------------------------------------------------------------------

    void schedIn_handler(FwIndexType portNum, U32 context) override;

    //! Cache LED state; portNum 0 = yellow, 1 = green, 2 = red
    Drv::GpioStatus setLed_handler(FwIndexType portNum, const Fw::Logic& state) override;

    //! Cache motor command; hardware write batched in next schedIn
    void setMotors_handler(FwIndexType portNum, const ROMI::MotorCommand& speed) override;

    // ----------------------------------------------------------------------
    // Command handler implementations
    // ----------------------------------------------------------------------

    void SET_YELLOW_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) override;

    void SET_GREEN_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) override;

    void SET_RED_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) override;

    //! Cache left/right motor velocities; flushed to hardware in next schedIn
    void SET_MOTORS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, I16 left, I16
 right) override;

    //! Cache a note sequence; the buzzer write is batched into next schedIn
    void PLAY_NOTES_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdStringArg& notes) override;

    // ----------------------------------------------------------------------
    // Private helpers
    // ----------------------------------------------------------------------

    //! Flush the cached command state then read the telemetry block.
    //! \return true on success, false if any I2C transaction fails.
    bool performI2cCycle(U8 i2cAddr);

    // ----------------------------------------------------------------------
    // Static I/O buffers (no heap allocation)
    // ----------------------------------------------------------------------

    //! Command write buffer: [reg=21, yellow, green, red, L_lo, L_hi, R_lo, R_hi]
    U8 m_cmdBuf[8];
    Fw::Buffer m_cmdFwBuf;

    //! Notes write buffer: [reg=28, playNotes=1, notes×14]
    U8 m_notesBuf[16];
    Fw::Buffer m_notesFwBuf;

    //! Single-byte register-address buffer used to start a telemetry read (reg 0)
    U8 m_regBuf[1];
    Fw::Buffer m_regFwBuf;

    //! 21-byte telemetry receive buffer (registers 0–20)
    U8 m_telemBuf[21];
    Fw::Buffer m_telemFwBuf;

    // ----------------------------------------------------------------------
    // Cached output state (flushed to hardware on every schedIn)
    // ----------------------------------------------------------------------

    //! Cached LED states: [0] = yellow, [1] = green, [2] = red (0 off, 1 on)
    U8 m_ledCache[3];

    //! Cached motor commands: [0] = left, [1] = right
    I16 m_motorCache[2];

    //! Pending note sequence flagged by PLAY_NOTES, flushed on next schedIn
    bool m_playNotesPending;

    //! Cached note sequence (NUL-padded) written to the Romi buzzer registers
    char m_notes[14];

    // ----------------------------------------------------------------------
    // Change-detection state
    // ----------------------------------------------------------------------

    //! Last known button states for A, B, C (0 = released, 1 = pressed)
    U8 m_lastButtonState[3];
};

}  // namespace ROMI

#endif  // ROMI_RomiHWDriver_HPP
