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

// -------------------------------------------------------------------------
// LSM6DS33 accelerometer/gyro (on-board IMU) configuration
//
// The LSM6DS33 is an independent slave on the same I2C bus (address 0x6B by
// default); RomiImu reads it directly through RomiI2cDriver.  These constants
// set the output data rate, full-scale ranges, and the corresponding LSB->
// engineering-unit scale factors.  Change CTRL1/CTRL2 *and* the matching scale
// factor together if you re-range the sensor.
// -------------------------------------------------------------------------

//! CTRL1_XL (0x10): accelerometer ODR=208 Hz, full-scale = +/-2 g.
constexpr U8 LSM6DS33_CTRL1_XL = 0x50;

//! CTRL2_G (0x11): gyro ODR=208 Hz, full-scale = +/-245 dps.
constexpr U8 LSM6DS33_CTRL2_G = 0x50;

//! CTRL3_C (0x12): BDU=1 (block data update, avoids torn LSB/MSB pairs) and
//! IF_INC=1 (register auto-increment, so one read sweeps the output block).
constexpr U8 LSM6DS33_CTRL3_C = 0x44;

//! Accelerometer sensitivity at +/-2 g: 0.061 mg/LSB -> g per LSB.
constexpr F32 LSM6DS33_ACCEL_SCALE_G = 0.000061f;

//! Gyro sensitivity at +/-245 dps: 8.75 mdps/LSB -> deg/s per LSB.
constexpr F32 LSM6DS33_GYRO_SCALE_DPS = 0.00875f;

//! Temperature: 16 LSB/degC, reference offset 25 degC (datasheet).
constexpr F32 LSM6DS33_TEMP_SCALE_C = 1.0f / 16.0f;
constexpr F32 LSM6DS33_TEMP_OFFSET_C = 25.0f;

}  // namespace ROMI

#endif  // ROMI_ROMICFG_HPP
