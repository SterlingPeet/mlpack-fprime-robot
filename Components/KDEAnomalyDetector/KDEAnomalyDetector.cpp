// ======================================================================
// \title  KDEAnomalyDetector.cpp
// \author ryan
// \brief  cpp file for KDEAnomalyDetector component implementation class
// ======================================================================

#include "Components/KDEAnomalyDetector/KDEAnomalyDetector.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

KDEAnomalyDetector ::KDEAnomalyDetector(const char* const compName) : KDEAnomalyDetectorComponentBase(compName) {
    // Create random data and build a tree.
    // TODO: actually do something reasonable!
    treeData.randu(5, 1000);
    tree = mlpack::KDTree<>(treeData);
}

KDEAnomalyDetector ::~KDEAnomalyDetector() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void KDEAnomalyDetector ::run_handler(FwIndexType portNum, U32 context) {
    // TODO: run anomaly detector
}

void KDEAnomalyDetector ::tlmIn_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    // TODO
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void KDEAnomalyDetector ::RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO: retrain the model
    this->tlmWrite_MODEL_DIMENSIONS(tree.Dataset().n_rows);
    this->tlmWrite_MODEL_POINTS(tree.Dataset().n_cols);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void KDEAnomalyDetector ::REPORT_DENSITY_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO: report the density of the most recent telemetry frame
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
