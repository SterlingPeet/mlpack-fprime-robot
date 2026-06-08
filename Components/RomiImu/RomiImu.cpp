// ======================================================================
// \title  RomiImu.cpp
// \brief  Driver for the Romi 32U4 on-board LSM6DS33 accelerometer/gyro
// ======================================================================

#include "Components/RomiImu/RomiImu.hpp"
#include "Components/RomiTypes/FppConstantsAc.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"
#include "Fw/Types/BasicTypes.hpp"
#include "config/RomiCfg.hpp"

#include <cstring>

namespace ROMI {

// -------------------------------------------------------------------------
// LSM6DS33 register addresses
// -------------------------------------------------------------------------

static constexpr U8 REG_WHO_AM_I = 0x0F;    // identity, returns 0x69
static constexpr U8 REG_CTRL1_XL = 0x10;    // accel ODR + full-scale
static constexpr U8 REG_CTRL2_G = 0x11;     // gyro ODR + full-scale
static constexpr U8 REG_CTRL3_C = 0x12;     // BDU + IF_INC (auto-increment)
static constexpr U8 REG_OUT_TEMP_L = 0x20;  // first output register read back

// Byte offsets within the 14-byte output block (== register - 0x20).
static constexpr FwSizeType OFF_TEMP = 0;
static constexpr FwSizeType OFF_GYRO_X = 2;
static constexpr FwSizeType OFF_GYRO_Y = 4;
static constexpr FwSizeType OFF_GYRO_Z = 6;
static constexpr FwSizeType OFF_ACCEL_X = 8;
static constexpr FwSizeType OFF_ACCEL_Y = 10;
static constexpr FwSizeType OFF_ACCEL_Z = 12;

static constexpr FwSizeType IMU_BLOCK_LEN = 14;

// -------------------------------------------------------------------------
// Little-endian read helper (matches RomiHWDriver)
// -------------------------------------------------------------------------

static I16 readI16LE(const U8* buf, FwSizeType off) {
    return static_cast<I16>(static_cast<U16>(buf[off]) | (static_cast<U16>(buf[off + 1]) << 8));
}

// -------------------------------------------------------------------------
// Construction and destruction
// -------------------------------------------------------------------------

RomiImu::RomiImu(const char* const compName) : RomiImuComponentBase(compName), m_configured(false) {
    std::memset(m_cfgBuf, 0, sizeof(m_cfgBuf));
    m_cfgFwBuf = Fw::Buffer(m_cfgBuf, sizeof(m_cfgBuf));

    m_regBuf[0] = REG_OUT_TEMP_L;
    m_regFwBuf = Fw::Buffer(m_regBuf, sizeof(m_regBuf));

    m_whoBuf[0] = 0;
    m_whoFwBuf = Fw::Buffer(m_whoBuf, sizeof(m_whoBuf));

    std::memset(m_imuBuf, 0, sizeof(m_imuBuf));
    m_imuFwBuf = Fw::Buffer(m_imuBuf, IMU_BLOCK_LEN);
}

RomiImu::~RomiImu() {}

// -------------------------------------------------------------------------
// Private helper: write a single configuration register
// -------------------------------------------------------------------------

bool RomiImu::writeReg(U8 i2cAddr, U8 reg, U8 value) {
    m_cfgBuf[0] = reg;
    m_cfgBuf[1] = value;
    const Drv::I2cStatus s = this->i2cWrite_out(0, i2cAddr, m_cfgFwBuf);
    if (s != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_ImuI2cError(s);
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------
// Private helper: verify identity and configure the sensor
// -------------------------------------------------------------------------

bool RomiImu::configure(U8 i2cAddr) {
    // 1. Read WHO_AM_I: write the register pointer, then read one byte.
    m_regBuf[0] = REG_WHO_AM_I;
    const Drv::I2cStatus s = this->i2cWriteRead_out(0, i2cAddr, m_regFwBuf, m_whoFwBuf);
    if (s != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_ImuI2cError(s);
        return false;
    }
    if (m_whoBuf[0] != LSM6DS33_WHO_AM_I_VALUE) {
        this->log_WARNING_HI_ImuWhoAmIError(LSM6DS33_WHO_AM_I_VALUE, m_whoBuf[0]);
        return false;
    }

    // 2. Configure output data rate, full-scale ranges, and BDU/auto-increment.
    if (!this->writeReg(i2cAddr, REG_CTRL1_XL, LSM6DS33_CTRL1_XL) ||
        !this->writeReg(i2cAddr, REG_CTRL2_G, LSM6DS33_CTRL2_G) ||
        !this->writeReg(i2cAddr, REG_CTRL3_C, LSM6DS33_CTRL3_C)) {
        return false;
    }

    this->log_ACTIVITY_HI_ImuInitOk();
    return true;
}

// -------------------------------------------------------------------------
// Private helper: read the 14-byte output block
// -------------------------------------------------------------------------

bool RomiImu::readBlock(U8 i2cAddr) {
    m_regBuf[0] = REG_OUT_TEMP_L;
    const Drv::I2cStatus s = this->i2cWriteRead_out(0, i2cAddr, m_regFwBuf, m_imuFwBuf);
    if (s != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_ImuI2cError(s);
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------
// schedIn_handler
// -------------------------------------------------------------------------

void RomiImu::schedIn_handler(FwIndexType portNum, U32 context) {
    Fw::ParamValid valid;
    const U8 i2cAddr = this->paramGet_I2C_ADDRESS(valid);

    // Lazily configure on the first cycle (and re-configure after any failure
    // that cleared m_configured), so the component recovers if the sensor was
    // briefly unreachable at start-up.
    if (!m_configured) {
        m_configured = this->configure(i2cAddr);
        if (!m_configured) {
            return;  // try again next cycle
        }
    }

    if (!this->readBlock(i2cAddr)) {
        m_configured = false;  // force re-init on the next cycle
        return;
    }

    // Parse little-endian raw counts and scale to engineering units.
    const F32 accelX = static_cast<F32>(readI16LE(m_imuBuf, OFF_ACCEL_X)) * LSM6DS33_ACCEL_SCALE_G;
    const F32 accelY = static_cast<F32>(readI16LE(m_imuBuf, OFF_ACCEL_Y)) * LSM6DS33_ACCEL_SCALE_G;
    const F32 accelZ = static_cast<F32>(readI16LE(m_imuBuf, OFF_ACCEL_Z)) * LSM6DS33_ACCEL_SCALE_G;

    const F32 gyroX = static_cast<F32>(readI16LE(m_imuBuf, OFF_GYRO_X)) * LSM6DS33_GYRO_SCALE_DPS;
    const F32 gyroY = static_cast<F32>(readI16LE(m_imuBuf, OFF_GYRO_Y)) * LSM6DS33_GYRO_SCALE_DPS;
    const F32 gyroZ = static_cast<F32>(readI16LE(m_imuBuf, OFF_GYRO_Z)) * LSM6DS33_GYRO_SCALE_DPS;

    const F32 tempC = static_cast<F32>(readI16LE(m_imuBuf, OFF_TEMP)) * LSM6DS33_TEMP_SCALE_C + LSM6DS33_TEMP_OFFSET_C;

    this->tlmWrite_AccelX(accelX);
    this->tlmWrite_AccelY(accelY);
    this->tlmWrite_AccelZ(accelZ);
    this->tlmWrite_GyroX(gyroX);
    this->tlmWrite_GyroY(gyroY);
    this->tlmWrite_GyroZ(gyroZ);
    this->tlmWrite_ImuTemp(tempC);
}

// -------------------------------------------------------------------------
// IMU_INIT command handler
// -------------------------------------------------------------------------

void RomiImu::IMU_INIT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    Fw::ParamValid valid;
    const U8 i2cAddr = this->paramGet_I2C_ADDRESS(valid);
    m_configured = this->configure(i2cAddr);
    this->cmdResponse_out(opCode, cmdSeq, m_configured ? Fw::CmdResponse::OK : Fw::CmdResponse::EXECUTION_ERROR);
}

}  // namespace ROMI
