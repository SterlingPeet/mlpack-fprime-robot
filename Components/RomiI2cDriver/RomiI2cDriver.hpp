// ======================================================================
// \title  RomiI2cDriver.hpp
// \brief  I2C driver for the Romi 32U4 PololuRPiSlave interface
//
// Uses write + delay + read instead of the atomic I2C_RDWR ioctl so that
// the PololuRPiSlave firmware has time to prepare read data between the
// register-address write and the subsequent data read.
// ======================================================================

#ifndef ROMI_RomiI2cDriver_HPP
#define ROMI_RomiI2cDriver_HPP

#include "Components/RomiI2cDriver/RomiI2cDriverComponentAc.hpp"

namespace ROMI {

class RomiI2cDriver final : public RomiI2cDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    explicit RomiI2cDriver(const char* const compName);

    ~RomiI2cDriver();

    // ----------------------------------------------------------------------
    // Public setup
    // ----------------------------------------------------------------------

    //! Open the I2C bus device file.  Must be called during topology setup
    //! before any I2C transactions are attempted.
    //! \param device  Path to the device node, e.g. "/dev/i2c-1"
    //! \return true on success; false on failure (I2cOpenError event emitted)
    bool open(const char* device);

  private:
    // ----------------------------------------------------------------------
    // Port handler implementations
    // ----------------------------------------------------------------------

    //! Write bytes to an I2C slave device.
    Drv::I2cStatus write_handler(FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) override;

    //! Read bytes from an I2C slave device.
    //! Prefer writeRead for Romi telemetry reads; this port does not send a
    //! register-address byte first.
    Drv::I2cStatus read_handler(FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) override;

    //! Write a register-address byte, wait I2C_READ_DELAY_US, then read data.
    //! This is the correct path for all Romi telemetry reads.
    Drv::I2cStatus writeRead_handler(FwIndexType portNum,
                                     U32 addr,
                                     Fw::Buffer& writeBuffer,
                                     Fw::Buffer& readBuffer) override;

    // ----------------------------------------------------------------------
    // Member data
    // ----------------------------------------------------------------------

    // Suppress unused-field warning when compiling the stub.
#ifndef STUBBED_ROMI_I2C_DRIVER
    int m_fd;  //!< I2C bus file descriptor; -1 when not open
#endif
};

}  // namespace ROMI

#endif  // ROMI_RomiI2cDriver_HPP
