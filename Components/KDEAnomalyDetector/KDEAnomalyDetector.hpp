// ======================================================================
// \title  KDEAnomalyDetector.hpp
// \author ryan
// \brief  hpp file for KDEAnomalyDetector component implementation class
// ======================================================================

#ifndef Components_KDEAnomalyDetector_HPP
#define Components_KDEAnomalyDetector_HPP

#include <mlpack.hpp>
#include "Components/KDEAnomalyDetector/KDEAnomalyDetectorComponentAc.hpp"

namespace Components {

class KDEAnomalyDetector final : public KDEAnomalyDetectorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct KDEAnomalyDetector object
    KDEAnomalyDetector(const char* const compName  //!< The component name
    );

    //! Destroy KDEAnomalyDetector object
    ~KDEAnomalyDetector();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for run
    void run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;

    //! Handler implementation for tlmIn
    void tlmIn_handler(FwIndexType portNum,  //!< The port number
                       FwChanIdType id,      //!< Telemetry Channel ID
                       Fw::Time& timeTag,    //!< Time Tag
                       Fw::TlmBuffer& val    //!< Buffer containing serialized telemetry value
                       ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command RESET
    void RESET_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                          U32 cmdSeq            //!< The command sequence number
                          ) override;

    //! Handler implementation for command REPORT_DENSITY
    void REPORT_DENSITY_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                   U32 cmdSeq            //!< The command sequence number
                                   ) override;

  private:
    //! The instantiated data we will build the tree on.
    arma::mat treeData;
    //! The current version of the tree.
    mlpack::KDTree<> tree;
};

}  // namespace Components

#endif
