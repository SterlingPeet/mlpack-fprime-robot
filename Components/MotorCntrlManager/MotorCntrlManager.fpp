module ROMI {

  @ Integrates raw encoder pulses into I32 absolute odometry and manages
  @ motor enable/disable state.
  @
  @ On every 50 Hz schedIn tick this component:
  @   1. Applies the cached motor setpoint to RomiHWDriver (zero when disabled).
  @   2. Pushes the most recent encoder deltas to PidCtrlManager.
  @   3. Emits odometry and status telemetry.
  @
  @ receiveEncoders is called by RomiHWDriver each I2C cycle.  It computes
  @ the per-cycle delta (with I16 wrap-around handling) and accumulates I32
  @ odometry independently of the schedIn rate.
  active component MotorCntrlManager {

    # ----------------------------------------------------------------------
    # Port connections
    # ----------------------------------------------------------------------

    @ Sched port driven by rateGroup4 at 50 Hz
    async input port schedIn: Svc.Sched

    @ Encoder values pushed by RomiHWDriver.sendEncoders each I2C cycle
    async input port receiveEncoders: MotorPairEncoders

    @ Motor setpoint forwarded by PidCtrlManager (or set via direct command)
    async input port setMotorSetpoint: MotorPairSpeed

    @ Forward the effective motor command to RomiHWDriver each schedIn tick
    output port setMotors: MotorPairSpeed

    @ Push per-cycle encoder deltas to PidCtrlManager for velocity/position PID
    output port sendEncoderDeltas: MotorPairEncoders

    # ----------------------------------------------------------------------
    # Commands
    # ----------------------------------------------------------------------

    @ Enable the drive motors (motors are disabled at startup)
    async command ENABLE_MOTORS opcode 0

    @ Disable the drive motors and zero the motor command immediately
    async command DISABLE_MOTORS opcode 1

    @ Reset the accumulated left and right odometry counters to zero
    async command RESET_ODOMETRY opcode 2

    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------

    @ Accumulated left-wheel odometry in encoder counts
    telemetry LeftOdometry: I32 id 0

    @ Accumulated right-wheel odometry in encoder counts
    telemetry RightOdometry: I32 id 1

    @ Current motor enable/disable state
    telemetry MotorsEnabled: Fw.On id 2

    @ Left encoder delta from the most recent receiveEncoders call
    telemetry LeftDelta: I16 id 3

    @ Right encoder delta from the most recent receiveEncoders call
    telemetry RightDelta: I16 id 4

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------

    @ Emitted when the motor enable state changes via command
    event MotorsStateChanged(
      $state: Fw.On @< New motor state
    ) severity activity high \
      id 0x00 \
      format "Motors {}"

    @ Emitted when odometry is reset by command
    event OdometryReset \
      severity activity high \
      id 0x01 \
      format "Odometry reset to zero"

    # ----------------------------------------------------------------------
    # Standard AC Ports
    # ----------------------------------------------------------------------

    time get port timeCaller
    command reg port cmdRegOut
    command recv port cmdIn
    command resp port cmdResponseOut
    text event port logTextOut
    event port logOut
    telemetry port tlmOut

  }

}
