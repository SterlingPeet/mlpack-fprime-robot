// ======================================================================
// \title  RomiCfg.hpp
// \brief  Project-level configuration constants for the Romi 32U4 interface
// ======================================================================

#ifndef ROMI_ROMICFG_HPP
#define ROMI_ROMICFG_HPP

#include "Fw/FPrimeBasicTypes.hpp"

namespace ROMI {

//! Delay in microseconds between writing a register address and reading back data.
//! The PololuRPiSlave library on the Romi 32U4 requires this gap to prepare
//! read data after receiving the register-address byte.  The Linux I2C_RDWR
//! atomic ioctl does not insert this gap, so it must be done manually.
constexpr U32 I2C_READ_DELAY_US = 100;

//! Default I2C bus device path for the Romi on a Raspberry Pi (bus 1).
constexpr const char* I2C_DEVICE = "/dev/i2c-1";

}  // namespace ROMI

#endif  // ROMI_ROMICFG_HPP
