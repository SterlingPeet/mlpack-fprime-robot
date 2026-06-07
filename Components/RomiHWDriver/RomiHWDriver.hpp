// ======================================================================
// \title  RomiHWDriver.hpp
// \brief  Hardware abstraction layer for the Romi 32U4 over I2C
//
// On every schedIn tick this component issues ONE I2C writeRead transaction:
//   write: [reg=0, pad×21, yellow, green, red, L_lo, L_hi, R_lo, R_hi] (29 bytes)
//   read:  28 bytes — the full PololuRPiSlave telemetry struct
// The 32U4 sketch unconditionally overwrites the padded (read-only) fields
// with fresh sensor data before every finalizeWrites(), so writing zeros
// there is safe.  setMotors and setLed only cache values; hardware I/O is
// batched into schedIn to keep I2C traffic deterministic at 50 Hz.
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

    // ----------------------------------------------------------------------
    // Private helpers
    // ----------------------------------------------------------------------

    //! Flush the cached write state then read the full telemetry struct.
    //! \return true on success, false if either I2C transaction fails.
    bool performI2cCycle(U8 i2cAddr);

    // ----------------------------------------------------------------------
    // Static I/O buffers (no heap allocation)
    // ----------------------------------------------------------------------

    //! Write buffer: [reg=0, pad×21, yellow, green, red, L_lo, L_hi, R_lo, R_hi]
    //! Bytes 1–21 are zero-padded (32U4 ignores writes to its sensor-owned fields).
    U8 m_writeBuf[29];
    Fw::Buffer m_writeFwBuf;

    //! 28-byte receive buffer (registers 0–27 from the PololuRPiSlave struct)
    U8 m_telemBuf[28];
    Fw::Buffer m_telemFwBuf;

    // ----------------------------------------------------------------------
    // Cached output state (flushed to hardware on every schedIn)
    // ----------------------------------------------------------------------

    //! Cached LED states: [0] = yellow, [1] = green, [2] = red (0 off, 1 on)
    U8 m_ledCache[3];

    //! Cached motor commands: [0] = left, [1] = right
    I16 m_motorCache[2];

    // ----------------------------------------------------------------------
    // Change-detection state
    // ----------------------------------------------------------------------

    //! Last known button states for A, B, C (0 = released, 1 = pressed)
    U8 m_lastButtonState[3];
};

}  // namespace ROMI

#endif  // ROMI_RomiHWDriver_HPP
