// ======================================================================
// \title  KDEAnomalyDetector.hpp
// \author ryan
// \brief  hpp file for KDEAnomalyDetector component implementation class
// ======================================================================

#ifndef Components_KDEAnomalyDetector_HPP
#define Components_KDEAnomalyDetector_HPP

#include <mlpack/core.hpp>
#include <mlpack/methods/kde.hpp>
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
    constexpr static const size_t numDims = 33;

    //! The current version of the tree.
    mlpack::KDE<> kde;
    //! Means of each feature in the current tree.
    arma::vec::fixed<numDims> dimMeans;
    //! Variances of each feature in the current tree.
    arma::vec::fixed<numDims> dimStddevs;

    //! Number of times the tree has been trained.
    U64 numResets;

    std::map<FwChanIdType, size_t> dimMap;
    std::array<size_t, numDims> dimTypes;

    // Note: this cache is built on the assumption that telemetry comes in
    // pretty much once per second!  This will still work if that's not true,
    // but the cache may grow pretty large.
    std::vector<std::map<Fw::Time, double>> tlmCache;
};

}  // namespace Components

#endif
