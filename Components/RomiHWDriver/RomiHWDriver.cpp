// ======================================================================
// \title  RomiHWDriver.cpp
// \author Sterling Peet <sterling.peet@gatech.edu>
// \brief  cpp file for RomiHWDriver component implementation class
// ======================================================================

#include "Components/RomiHWDriver/RomiHWDriver.hpp"

namespace ROMI {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

RomiHWDriver ::RomiHWDriver(const char* const compName) : RomiHWDriverComponentBase(compName) {
    this->m_lastI2cStatus = Drv::GpioStatus::OPEN_ERR;  // Default to uninitialized, schedIn will get an update
}

RomiHWDriver ::~RomiHWDriver() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void RomiHWDriver ::schedIn_handler(FwIndexType portNum, U32 context) {}

Drv::GpioStatus RomiHWDriver ::setLed_handler(FwIndexType portNum, const Fw::Logic& state) {
    // TODO return
}

void RomiHWDriver ::setMotors_handler(FwIndexType portNum, const ROMI::MotorCommand& speed) {
    // TODO
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void RomiHWDriver ::SET_YELLOW_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RomiHWDriver ::SET_GREEN_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RomiHWDriver ::SET_RED_LED_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On state) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ----------------------------------------------------------------------
// Handler implementations for user-defined internal interfaces
// ----------------------------------------------------------------------

void RomiHWDriver ::I2cCommand_internalInterfaceHandler(const Fw::Buffer& cmdBuff) {
    // TODO
}

}  // namespace ROMI
