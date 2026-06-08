module ROMI {
    @ Driver for the Romi 32U4 Control Board's on-board LSM6DS33 IMU
    @ (3-axis accelerometer + 3-axis gyro).
    @
    @ The LSM6DS33 is wired onto the same I2C bus the Raspberry Pi masters (the
    @ 3.3 V side, through the board's level shifters) as an independent slave at
    @ address 0x6B -- distinct from the 32U4 register-file slave at 0x14.  This
    @ component therefore talks to the sensor *directly* through the existing
    @ RomiI2cDriver; the 32U4 firmware is never involved.
    @
    @ On each schedIn it writes the output-register pointer (OUT_TEMP_L) and
    @ reads back the 14-byte block (temperature, gyro X/Y/Z, accel X/Y/Z) in a
    @ single writeRead, then scales the raw counts to engineering units.
    active component RomiImu {

        # ----------------------------------------------------------------------
        # General Ports
        # ----------------------------------------------------------------------

        @ Sched port: read the IMU and emit telemetry (wire to a 1 Hz rate group)
        async input port schedIn: Svc.Sched

        @ I2C write port: configuration-register writes and the output-register
        @ pointer set, of the form [reg, value...].  Wired to RomiI2cDriver.
        output port i2cWrite: Drv.I2c

        @ I2C writeRead port: writes the output-register address then reads back
        @ the sensor data block (discrete write + read, no repeated start).
        output port i2cWriteRead: Drv.I2cWriteRead

        # ----------------------------------------------------------------------
        # Commands
        # ----------------------------------------------------------------------

        @ Re-run the LSM6DS33 configuration sequence (WHO_AM_I check + CTRL
        @ register setup).  Also performed lazily on the first schedIn.
        async command IMU_INIT opcode 0

        # ----------------------------------------------------------------------
        # Telemetry
        # ----------------------------------------------------------------------

        @ Acceleration along the X axis
        telemetry AccelX: F32 id 0 format "{.4f} g"

        @ Acceleration along the Y axis
        telemetry AccelY: F32 id 1 format "{.4f} g"

        @ Acceleration along the Z axis
        telemetry AccelZ: F32 id 2 format "{.4f} g"

        @ Angular rate about the X axis
        telemetry GyroX: F32 id 3 format "{.2f} dps"

        @ Angular rate about the Y axis
        telemetry GyroY: F32 id 4 format "{.2f} dps"

        @ Angular rate about the Z axis
        telemetry GyroZ: F32 id 5 format "{.2f} dps"

        @ On-die temperature of the IMU
        telemetry ImuTemp: F32 id 6 format "{.1f} C"

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ The LSM6DS33 was found and configured successfully
        event ImuInitOk severity activity high \
          id 0 \
          format "LSM6DS33 IMU initialised"

        @ WHO_AM_I did not return the expected identity byte (sensor missing,
        @ wrong address, or bus fault)
        event ImuWhoAmIError(
          expected: U8 @< Expected WHO_AM_I value
          got: U8      @< Value actually read back
        ) severity warning high \
          id 1 \
          format "LSM6DS33 WHO_AM_I mismatch: expected {x}, got {x}"

        @ An I2C transaction to the IMU failed
        event ImuI2cError(
          status: Drv.I2cStatus @< Specific I2C error code
        ) severity warning high \
          id 2 \
          format "LSM6DS33 I2C transaction failed with status {}" \
          throttle 5

        # ----------------------------------------------------------------------
        # Parameters
        # ----------------------------------------------------------------------

        @ I2C address of the LSM6DS33 (default 0x6B)
        param I2C_ADDRESS: U8 default LSM6DS33_I2C_DEFAULT_ADDRESS id 0

        # ----------------------------------------------------------------------
        # Standard AC Ports
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

        @ Port to set the value of a parameter
        param set port prmSetOut

    }
}
