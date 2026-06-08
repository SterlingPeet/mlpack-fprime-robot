// ======================================================================
// \title  MotorCntrlManager.hpp
// \brief  Motor controller: encoder delta integration, odometry
//         accumulation, and motor enable/disable management.
//
// receiveEncoders is called by RomiHWDriver on every I2C cycle.  It
// computes the signed per-cycle delta (with I16 wrap-around tolerance)
// and accumulates it into I32 absolute odometry.
//
// schedIn (50 Hz) applies the cached motor setpoint to RomiHWDriver and
// pushes encoder deltas to PidCtrlManager for closed-loop control.
// Motors default to DISABLED at construction and must be explicitly
// enabled via the ENABLE_MOTORS command.
// ======================================================================

#ifndef ROMI_MotorCntrlManager_HPP
#define ROMI_MotorCntrlManager_HPP

#include "Components/MotorCntrlManager/MotorCntrlManagerComponentAc.hpp"
#include "Fw/Types/BasicTypes.hpp"

namespace ROMI {

class MotorCntrlManager : public MotorCntrlManagerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    explicit MotorCntrlManager(const char* const compName);
    ~MotorCntrlManager();

  private:
    // ----------------------------------------------------------------------
    // Port handler implementations
    // ----------------------------------------------------------------------

    //! Emit telemetry, apply cached motor command, push encoder deltas
    void schedIn_handler(FwIndexType portNum, U32 context) override;

    //! Emit computed velocity telemetry
    void schedInSlow_handler(FwIndexType portNum, U32 context) override;

    //! Compute delta (wrap-safe), accumulate I32 odometry
    void receiveEncoders_handler(FwIndexType portNum, const ROMI::MotorEncoders& encoders) override;

    //! Cache the motor setpoint for application on the next schedIn tick
    void setMotorSetpoint_handler(FwIndexType portNum, const ROMI::MotorCommand& speed) override;

    // ----------------------------------------------------------------------
    // Command handler implementations
    // ----------------------------------------------------------------------

    //! Handler implementation for command ENABLE_MOTORS
    //!
    //! Enable the drive motors (motors are disabled at startup)
    void ENABLE_MOTORS_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                  U32 cmdSeq,           //!< The command sequence number
                                  I16 left,             //!< Left wheel velocity
                                  I16 right             //!< Right wheel velocity
                                  ) override;

    //! Handler implementation for command DISABLE_MOTORS
    //!
    //! Disable the drive motors and zero the motor command immediately
    void DISABLE_MOTORS_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                   U32 cmdSeq            //!< The command sequence number
                                   ) override;

    //! Handler implementation for command RESET_ODOMETRY
    //!
    //! Reset the accumulated left and right odometry counters to zero
    void RESET_ODOMETRY_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                   U32 cmdSeq            //!< The command sequence number
                                   ) override;

    // ----------------------------------------------------------------------
    // Private helpers
    // ----------------------------------------------------------------------

    //! Compute signed delta between two I16 encoder readings, tolerating
    //! wrap-around at ±32767.  Correct as long as motion between samples
    //! stays within ±32767 counts (impossible at 50 Hz for this platform).
    static I16 computeDelta(I16 current, I16 previous);

    // ----------------------------------------------------------------------
    // State
    // ----------------------------------------------------------------------

    //! Last raw encoder values; used as the baseline for delta computation
    I16 m_lastLeft;
    I16 m_lastRight;

    //! False until the first receiveEncoders call; prevents a spurious spike
    bool m_encodersInitialized;

    //! Accumulated absolute odometry (encoder counts)
    I32 m_leftOdo;
    I32 m_rightOdo;

    //! Per-cycle encoder deltas cached by receiveEncoders for schedIn use
    I16 m_leftDelta;
    I16 m_rightDelta;

    //! Cached motor setpoint applied each schedIn (zeroed when disabled)
    I16 m_cmdLeft;
    I16 m_cmdRight;

    //! Motor enable state — false at construction, toggled by commands
    bool m_motorsEnabled;

    //! Last odometry reading for velocity computation.
    I32 m_lastLeftOdo;
    I32 m_lastRightOdo;

    //! Time of last odometry reading.
    Fw::Time m_lastOdoTime;
};

}  // namespace ROMI

#endif  // ROMI_MotorCntrlManager_HPP
