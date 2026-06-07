// ======================================================================
// \title  RomiI2cDriverStub.cpp
// \brief  Stub implementation for host (non-Linux) builds.
//         All handlers return I2C_OK; open() always succeeds.
// ======================================================================

#include "Components/RomiI2cDriver/RomiI2cDriver.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"

namespace ROMI {

RomiI2cDriver::RomiI2cDriver(const char* const compName) : RomiI2cDriverComponentBase(compName) {}

RomiI2cDriver::~RomiI2cDriver() {}

bool RomiI2cDriver::open(const char* device) {
    return true;
}

Drv::I2cStatus RomiI2cDriver::write_handler(FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) {
    return Drv::I2cStatus::I2C_OK;
}

Drv::I2cStatus RomiI2cDriver::read_handler(FwIndexType portNum, U32 addr, Fw::Buffer& serBuffer) {
    return Drv::I2cStatus::I2C_OK;
}

Drv::I2cStatus RomiI2cDriver::writeRead_handler(FwIndexType portNum,
                                                U32 addr,
                                                Fw::Buffer& writeBuffer,
                                                Fw::Buffer& readBuffer) {
    return Drv::I2cStatus::I2C_OK;
}

}  // namespace ROMI
