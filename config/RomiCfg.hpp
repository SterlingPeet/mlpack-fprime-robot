// ======================================================================
// \title  RomiCfg.hpp
// \brief  Project-level configuration constants for the Romi 32U4 interface
// ======================================================================

#ifndef ROMI_ROMICFG_HPP
#define ROMI_ROMICFG_HPP

#include "Fw/FPrimeBasicTypes.hpp"

namespace ROMI {

//! Delay in microseconds between writing a register address and reading back data.
//! Gives the Romi firmware loop() time to refresh its shared buffer before the
//! read, and separates the two transactions with a real STOP (the Pi BCM I2C
//! peripheral mishandles the repeated start the atomic I2C_RDWR ioctl emits).
//! Tunable: with the vanilla-Wire firmware this can likely drop toward ~50 us.
constexpr U32 I2C_READ_DELAY_US = 100;

//! Default I2C bus device path for the Romi on a Raspberry Pi (bus 1).
constexpr const char* I2C_DEVICE = "/dev/i2c-1";

}  // namespace ROMI

#endif  // ROMI_ROMICFG_HPP
