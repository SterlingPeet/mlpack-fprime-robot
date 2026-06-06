// ======================================================================
// \title  TlmSplitter.cpp
// \author ryan
// \brief  cpp file for TlmSplitter component implementation class
// ======================================================================

#include "Components/TlmSplitter/TlmSplitter.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

TlmSplitter ::TlmSplitter(const char* const compName) : TlmSplitterComponentBase(compName) {}

TlmSplitter ::~TlmSplitter() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void TlmSplitter ::TlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    if (this->isConnected_TlmOutA_OutputPort(0)) {
        Fw::Time timeTagA = timeTag;
        Fw::TlmBuffer valA = val;
        this->TlmOutA_out(0, id, timeTagA, valA);
    }
    if (this->isConnected_TlmOutB_OutputPort(0)) {
        Fw::Time timeTagB = timeTag;
        Fw::TlmBuffer valB = val;
        this->TlmOutB_out(0, id, timeTagB, valB);
    }
}

}  // namespace Components
