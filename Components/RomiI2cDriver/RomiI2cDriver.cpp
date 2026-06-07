// ======================================================================
// \title  RomiI2cDriver.cpp
// \brief  I2C driver for the Romi 32U4 PololuRPiSlave interface
// ======================================================================

#include "Components/RomiI2cDriver/RomiI2cDriver.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"
#include "Fw/Types/Assert.hpp"
#include "Fw/Types/String.hpp"
#include "config/RomiCfg.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace ROMI {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

RomiI2cDriver::RomiI2cDriver(const char* const compName) : RomiI2cDriverComponentBase(compName), m_fd(-1) {}

RomiI2cDriver::~RomiI2cDriver() {
    if (m_fd != -1) {
        ::close(m_fd);
    }
}

// ----------------------------------------------------------------------
// Public setup
// ----------------------------------------------------------------------

bool RomiI2cDriver::open(const char* device) {
    FW_ASSERT(device != nullptr);
    m_fd = ::open(device, O_RDWR);
    if (m_fd == -1) {
        this->log_WARNING_HI_I2cOpenError(Fw::String(device));
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------
// Port handler implementations
// ----------------------------------------------------------------------

Drv::I2cStatus RomiI2cDriver::write_handler(FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) {
    if (m_fd == -1) {
        return Drv::I2cStatus::I2C_OPEN_ERR;
    }
    if (ioctl(m_fd, I2C_SLAVE, addr) == -1) {
        return Drv::I2cStatus::I2C_ADDRESS_ERR;
    }
    FW_ASSERT(serBuffer.getData() != nullptr);
    FW_ASSERT_NO_OVERFLOW(serBuffer.getSize(), size_t);
    if (::write(m_fd, serBuffer.getData(), static_cast<size_t>(serBuffer.getSize())) !=
        static_cast<ssize_t>(serBuffer.getSize())) {
        return Drv::I2cStatus::I2C_WRITE_ERR;
    }
    return Drv::I2cStatus::I2C_OK;
}

Drv::I2cStatus RomiI2cDriver::read_handler(FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) {
    if (m_fd == -1) {
        return Drv::I2cStatus::I2C_OPEN_ERR;
    }
    if (ioctl(m_fd, I2C_SLAVE, addr) == -1) {
        return Drv::I2cStatus::I2C_ADDRESS_ERR;
    }
    FW_ASSERT(serBuffer.getData() != nullptr);
    FW_ASSERT_NO_OVERFLOW(serBuffer.getSize(), size_t);
    if (::read(m_fd, serBuffer.getData(), static_cast<size_t>(serBuffer.getSize())) !=
        static_cast<ssize_t>(serBuffer.getSize())) {
        return Drv::I2cStatus::I2C_READ_ERR;
    }
    return Drv::I2cStatus::I2C_OK;
}

Drv::I2cStatus RomiI2cDriver::writeRead_handler(FwIndexType portNum,
                                                U32 addr,
                                                Fw::Buffer& writeBuffer,
                                                Fw::Buffer& readBuffer) {
    if (m_fd == -1) {
        return Drv::I2cStatus::I2C_OPEN_ERR;
    }
    if (ioctl(m_fd, I2C_SLAVE, addr) == -1) {
        return Drv::I2cStatus::I2C_ADDRESS_ERR;
    }

    FW_ASSERT(writeBuffer.getData() != nullptr);
    FW_ASSERT(readBuffer.getData() != nullptr);
    FW_ASSERT_NO_OVERFLOW(writeBuffer.getSize(), size_t);
    FW_ASSERT_NO_OVERFLOW(readBuffer.getSize(), size_t);

    // Step 1: write the register-address byte (or initial payload).
    if (::write(m_fd, writeBuffer.getData(), static_cast<size_t>(writeBuffer.getSize())) !=
        static_cast<ssize_t>(writeBuffer.getSize())) {
        return Drv::I2cStatus::I2C_WRITE_ERR;
    }

    // Step 2: wait for the PololuRPiSlave firmware to prepare read data.
    // The atomic I2C_RDWR ioctl does not insert this gap and causes the
    // 32U4 to return stale or garbled data.
    ::usleep(ROMI::I2C_READ_DELAY_US);

    // Step 3: read the response.
    if (::read(m_fd, readBuffer.getData(), static_cast<size_t>(readBuffer.getSize())) !=
        static_cast<ssize_t>(readBuffer.getSize())) {
        return Drv::I2cStatus::I2C_READ_ERR;
    }

    return Drv::I2cStatus::I2C_OK;
}

}  // namespace ROMI
