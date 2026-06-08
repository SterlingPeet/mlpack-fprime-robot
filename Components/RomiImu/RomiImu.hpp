// ======================================================================
// \title  RomiImu.hpp
// \brief  Driver for the Romi 32U4 on-board LSM6DS33 accelerometer/gyro
//
// The LSM6DS33 is an independent I2C slave (address 0x6B) on the same bus the
// Raspberry Pi masters, separate from the 32U4 register-file slave (0x14).
// This component reads it directly through RomiI2cDriver.  On the first
// schedIn (or on the IMU_INIT command) it verifies WHO_AM_I and writes the
// CTRL registers; on every schedIn it writes the output-register pointer and
// reads the 14-byte block [temp, gyro XYZ, accel XYZ], then scales the raw
// little-endian counts to g / deg-s / degC and emits them as telemetry.
// ======================================================================

#ifndef ROMI_RomiImu_HPP
#define ROMI_RomiImu_HPP

#include "Components/RomiImu/RomiImuComponentAc.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"
#include "Fw/Buffer/Buffer.hpp"
#include "Fw/Types/BasicTypes.hpp"

namespace ROMI {

class RomiImu : public RomiImuComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    explicit RomiImu(const char* const compName);
    ~RomiImu();

  private:
    // ----------------------------------------------------------------------
    // Port handler implementations
    // ----------------------------------------------------------------------

    //! Read the IMU and emit scaled telemetry; lazily configures on first call
    void schedIn_handler(FwIndexType portNum, U32 context) override;

    // ----------------------------------------------------------------------
    // Command handler implementations
    // ----------------------------------------------------------------------

    //! Re-run the configuration sequence on demand
    void IMU_INIT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    // ----------------------------------------------------------------------
    // Private helpers
    // ----------------------------------------------------------------------

    //! Verify WHO_AM_I and write the CTRL registers.
    //! \return true if the sensor responded with the expected identity and all
    //!         configuration writes succeeded.
    bool configure(U8 i2cAddr);

    //! Write a single configuration register: [reg, value].
    //! \return true on I2C_OK.
    bool writeReg(U8 i2cAddr, U8 reg, U8 value);

    //! Read the 14-byte output block starting at OUT_TEMP_L into m_imuBuf.
    //! \return true on I2C_OK.
    bool readBlock(U8 i2cAddr);

    // ----------------------------------------------------------------------
    // Static I/O buffers (no heap allocation)
    // ----------------------------------------------------------------------

    //! Two-byte configuration-write buffer: [reg, value]
    U8 m_cfgBuf[2];
    Fw::Buffer m_cfgFwBuf;

    //! Single-byte register-pointer buffer used to start a read
    U8 m_regBuf[1];
    Fw::Buffer m_regFwBuf;

    //! Single-byte WHO_AM_I receive buffer
    U8 m_whoBuf[1];
    Fw::Buffer m_whoFwBuf;

    //! 14-byte output block: temp(2) + gyro XYZ(6) + accel XYZ(6)
    U8 m_imuBuf[14];
    Fw::Buffer m_imuFwBuf;

    // ----------------------------------------------------------------------
    // State
    // ----------------------------------------------------------------------

    //! True once WHO_AM_I has matched and the CTRL registers are written
    bool m_configured;
};

}  // namespace ROMI

#endif  // ROMI_RomiImu_HPP
