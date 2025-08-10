module ROMI {
    @ Driver for low level control of the ROMI Robot.
    active component RomiHWDriver {

        # ----------------------------------------------------------------------
        # General Ports
        # ----------------------------------------------------------------------

        @ Sched port: Read Romi and emit telemetry
        async input port schedIn: Svc.Sched

        @ Set the motor speed
        async input port setMotors: MotorPairSpeed

        @ Set the state of an LED
        sync input port setLed: [3] Drv.GpioWrite

        @ Direct report of the motor encoders
        @ This output is optional and telemetry of the raw encoder values
        @ will be reported either way
        output port sendEncoders: MotorPairEncoders

        @ Send button changes to a listener
        @ When connected, only the listener will be notified, otherwise
        @ an event will be emitted to indicate the botton press to the
        @ operator via GDS.
        output port sendButton: [3] Drv.GpioRead

        # ----------------------------------------------------------------------
        # Commands
        # ----------------------------------------------------------------------

        @ Set the yellow LED
        async command SET_YELLOW_LED(
            $state: Fw.On @< New LED State
          ) opcode 0

        @ Set the green LED
        async command SET_GREEN_LED(
            $state: Fw.On @< New LED State
          ) opcode 1

        @ Set the red LED
        async command SET_RED_LED(
            $state: Fw.On @< New LED State
          ) opcode 2

        # ----------------------------------------------------------------------
        # Telemetry
        # ----------------------------------------------------------------------

        @ I2C Address of the Romi microcontoller
        telemetry i2cAddress: U8 id 0

        @ Battery voltage from the Romi battery pack
        telemetry BatteryVoltage: F32 id 1 \
          format "{.3f} V"

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ Button activity when the GPIO is not connected
        event ButtonStateChange(
          button: RomiButton @< Which button was changed
          $state: ButtonState @< The new button state
        ) severity activity high \
          id 0x00 \
          format "Romi button {} changed to {}"

        # ----------------------------------------------------------------------
        # Parameters
        # ----------------------------------------------------------------------

        @ Default I2C address for the Romi microcontoller
        param I2C_ADDRESS: U8 default ROMI_I2C_DEFAULT_ADDRESS id 0

        # ----------------------------------------------------------------------
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters
        # ----------------------------------------------------------------------

        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

        # ----------------------------------------------------------------------
        # Internal Interfaces (FPP Internal Ports)
        # ----------------------------------------------------------------------

        @ Internal helper for sending I2C command transactions
        internal port I2cCommand(
            cmdBuff: Fw.Buffer  @< Data to send to I2c Device
        )

    }
}
