// ======================================================================
// \title  TlmSplitter.hpp
// \author ryan
// \brief  hpp file for TlmSplitter component implementation class
// ======================================================================

#ifndef Components_TlmSplitter_HPP
#define Components_TlmSplitter_HPP

#include "Components/TlmSplitter/TlmSplitterComponentAc.hpp"

namespace Components {

class TlmSplitter final : public TlmSplitterComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct TlmSplitter object
    TlmSplitter(const char* const compName  //!< The component name
    );

    //! Destroy TlmSplitter object
    ~TlmSplitter();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for TlmRecv
    //!
    //! Telemetry input
    void TlmRecv_handler(FwIndexType portNum,  //!< The port number
                         FwChanIdType id,      //!< Telemetry Channel ID
                         Fw::Time& timeTag,    //!< Time Tag
                         Fw::TlmBuffer& val    //!< Buffer containing serialized telemetry value
                         ) override;
};

}  // namespace Components

#endif
