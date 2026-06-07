module ROMI {

  @ Custom I2C driver for the Romi 32U4 microcontroller.
  @
  @ The PololuRPiSlave library on the Romi 32U4 does not properly support
  @ the Linux I2C_RDWR ioctl (atomic repeated-start combined write+read).
  @ This component instead performs a discrete write, waits I2C_READ_DELAY_US
  @ (see config/RomiCfg.hpp), then reads — matching the behaviour of the
  @ original cFS romimot_hw driver.
  @
  @ The port interface mirrors Drv.LinuxI2cDriver so either driver can be
  @ wired into a topology targeting the Romi I2C bus.
  passive component RomiI2cDriver {

    # ----------------------------------------------------------------------
    # I2C ports  (same surface as Drv.LinuxI2cDriver / Drv.I2c interface)
    # ----------------------------------------------------------------------

    @ Write data to an I2C slave device.
    guarded input port write: Drv.I2c

    @ Read data from an I2C slave device.
    @ Note: for Romi telemetry reads always use writeRead so that the
    @ register-address byte is sent first.
    guarded input port read: Drv.I2c

    @ Write a register-address byte, wait I2C_READ_DELAY_US, then read data.
    @ Uses discrete write + read syscalls instead of I2C_RDWR to satisfy the
    @ PololuRPiSlave timing requirement.
    guarded input port writeRead: Drv.I2cWriteRead

    # ----------------------------------------------------------------------
    # Standard AC ports
    # ----------------------------------------------------------------------

    @ Port for requesting the current time
    time get port timeCaller

    @ Port for sending events to downlink
    event port logOut

    @ Port for sending textual representation of events
    text event port logTextOut

    # ----------------------------------------------------------------------
    # Events
    # ----------------------------------------------------------------------

    @ Failed to open the I2C device file during topology setup
    event I2cOpenError(
        device: string size 40 @< Device path that failed to open
    ) severity warning high \
      id 0 \
      format "Failed to open I2C device {}"

    @ write() syscall failed during a write or writeRead transaction
    event I2cWriteError(
        addr: U32 @< I2C slave address
        status: Drv.I2cStatus @< Specific error code
    ) severity warning high \
      id 1 \
      format "I2C write to address {} failed with status {}"

    @ read() syscall failed during a read or writeRead transaction
    event I2cReadError(
        addr: U32 @< I2C slave address
        status: Drv.I2cStatus @< Specific error code
    ) severity warning high \
      id 2 \
      format "I2C read from address {} failed with status {}"

  }

}
