// ======================================================================
// \title  RomiHWDriver.hpp
// \author Sterling Peet <sterling.peet@gatech.edu>
// \brief  hpp file for RomiHWDriver component implementation class
// ======================================================================

#ifndef ROMI_RomiHWDriver_HPP
#define ROMI_RomiHWDriver_HPP

#include "Components/RomiHWDriver/RomiHWDriverComponentAc.hpp"
#include "Components/RomiTypes/RomiTelemetrySerializableAc.hpp"
#include "Drv/Ports/I2cStatusEnumAc.hpp"
#include "Fw/Types/BasicTypes.hpp"

namespace ROMI {

class RomiHWDriver : public RomiHWDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct RomiHWDriver object
    RomiHWDriver(const char* const compName  //!< The component name
    );

    //! Destroy RomiHWDriver object
    ~RomiHWDriver();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for schedIn
    //!
    //! Sched port: Read Romi and emit telemetry
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;

    //! Handler implementation for setLed
    //!
    //! Set the state of an LED
    Drv::GpioStatus setLed_handler(FwIndexType portNum,  //!< The port number
                                   const Fw::Logic& state) override;

    //! Handler implementation for setMotors
    //!
    //! Set the motor speed
    void setMotors_handler(FwIndexType portNum,             //!< The port number
                           const ROMI::MotorCommand& speed  //!< Speed of the left and right motors
                           ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command SET_YELLOW_LED
    //!
    //! Set the yellow LED
    void SET_YELLOW_LED_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                   U32 cmdSeq,           //!< The command sequence number
                                   Fw::On state          //!< New LED State
                                   ) override;

    //! Handler implementation for command SET_GREEN_LED
    //!
    //! Set the green LED
    void SET_GREEN_LED_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                  U32 cmdSeq,           //!< The command sequence number
                                  Fw::On state          //!< New LED State
                                  ) override;

    //! Handler implementation for command SET_RED_LED
    //!
    //! Set the red LED
    void SET_RED_LED_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                U32 cmdSeq,           //!< The command sequence number
                                Fw::On state          //!< New LED State
                                ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined internal interfaces
    // ----------------------------------------------------------------------

    //! Handler implementation for I2cCommand
    //!
    //! Internal helper for sending I2C command transactions
    void I2cCommand_internalInterfaceHandler(const Fw::Buffer& cmdBuff  //!< Data to send to I2c Device
                                             ) override;

    // ----------------------------------------------------------------------
    // Member variables for component instance scope
    // ----------------------------------------------------------------------

    U8* m_i2cCmdBuff;
    Fw::Buffer m_i2cCmdBuffer;

    U8* m_i2cTelemBuff;
    Fw::Buffer m_i2cTelemBuffer;
    RomiTelemetry m_i2cTelem;
    Drv::I2cStatus m_lastI2cStatus;
};

}  // namespace ROMI

#endif
