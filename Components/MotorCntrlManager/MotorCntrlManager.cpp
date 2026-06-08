// ======================================================================
// \title  MotorCntrlManager.cpp
// ======================================================================

#include "Components/MotorCntrlManager/MotorCntrlManager.hpp"

namespace ROMI {

// -------------------------------------------------------------------------
// Construction and destruction
// -------------------------------------------------------------------------

MotorCntrlManager::MotorCntrlManager(const char* const compName)
    : MotorCntrlManagerComponentBase(compName),
      m_lastLeft(0),
      m_lastRight(0),
      m_encodersInitialized(false),
      m_leftOdo(0),
      m_rightOdo(0),
      m_leftDelta(0),
      m_rightDelta(0),
      m_cmdLeft(0),
      m_cmdRight(0),
      m_motorsEnabled(false) {}

MotorCntrlManager::~MotorCntrlManager() {}

// -------------------------------------------------------------------------
// Private helpers
// -------------------------------------------------------------------------

I16 MotorCntrlManager::computeDelta(I16 current, I16 previous) {
    // Cast through U16 so that subtraction wraps correctly at ±32767
    // without invoking signed-integer overflow (undefined behaviour).
    return static_cast<I16>(static_cast<U16>(current) - static_cast<U16>(previous));
}

// -------------------------------------------------------------------------
// schedIn_handler — 50 Hz tick
// -------------------------------------------------------------------------

void MotorCntrlManager::schedIn_handler(FwIndexType portNum, U32 context) {
    // Compute the effective motor command: zero when motors are disabled.
    const I16 cmdLeft = m_motorsEnabled ? m_cmdLeft : static_cast<I16>(0);
    const I16 cmdRight = m_motorsEnabled ? m_cmdRight : static_cast<I16>(0);

    // Apply motor command to RomiHWDriver every cycle so that disabling
    // motors actively drives the command to zero rather than just stopping
    // updates.
    if (this->isConnected_setMotors_OutputPort(0)) {
        ROMI::MotorCommand cmd(cmdLeft, cmdRight);
        this->setMotors_out(0, cmd);
    }

    // Push the latest encoder deltas to PidCtrlManager.
    if (this->isConnected_sendEncoderDeltas_OutputPort(0)) {
        ROMI::MotorEncoders deltas(m_leftDelta, m_rightDelta);
        this->sendEncoderDeltas_out(0, deltas);
    }

    // Emit telemetry channels.
    this->tlmWrite_LeftOdometry(m_leftOdo);
    this->tlmWrite_RightOdometry(m_rightOdo);
    this->tlmWrite_MotorsEnabled(m_motorsEnabled ? Fw::On::ON : Fw::On::OFF);
    this->tlmWrite_LeftDelta(m_leftDelta);
    this->tlmWrite_RightDelta(m_rightDelta);
}

// -------------------------------------------------------------------------
// receiveEncoders_handler — called by RomiHWDriver each I2C cycle
// -------------------------------------------------------------------------

void MotorCntrlManager::receiveEncoders_handler(FwIndexType portNum, const ROMI::MotorEncoders& encoders) {
    if (!m_encodersInitialized) {
        // Seed the baseline on the very first reading to avoid a spurious
        // large delta on startup.
        m_lastLeft = encoders.get_left();
        m_lastRight = encoders.get_right();
        m_encodersInitialized = true;
        return;
    }

    m_leftDelta = computeDelta(encoders.get_left(), m_lastLeft);
    m_rightDelta = computeDelta(encoders.get_right(), m_lastRight);

    m_leftOdo += static_cast<I32>(m_leftDelta);
    m_rightOdo += static_cast<I32>(m_rightDelta);

    m_lastLeft = encoders.get_left();
    m_lastRight = encoders.get_right();
}

// -------------------------------------------------------------------------
// setMotorSetpoint_handler — called by PidCtrlManager
// -------------------------------------------------------------------------

void MotorCntrlManager::setMotorSetpoint_handler(FwIndexType portNum, const ROMI::MotorCommand& speed) {
    m_cmdLeft = speed.get_left();
    m_cmdRight = speed.get_right();
}

// -------------------------------------------------------------------------
// Command handlers
// -------------------------------------------------------------------------

void MotorCntrlManager::ENABLE_MOTORS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, I16 left, I16 right) {
    m_cmdLeft = left;
    m_cmdRight = right;
    if (!m_motorsEnabled) {
        m_motorsEnabled = true;
        this->log_ACTIVITY_HI_MotorsStateChanged(Fw::On::ON);
    }
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void MotorCntrlManager::DISABLE_MOTORS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    if (m_motorsEnabled) {
        m_motorsEnabled = false;
        // Clear the cached setpoint so a re-enable doesn't apply stale speed.
        // m_cmdLeft = 0;
        // m_cmdRight = 0;
        this->log_ACTIVITY_HI_MotorsStateChanged(Fw::On::OFF);
    }
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void MotorCntrlManager::RESET_ODOMETRY_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_leftOdo = 0;
    m_rightOdo = 0;
    this->log_ACTIVITY_HI_OdometryReset();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace ROMI
