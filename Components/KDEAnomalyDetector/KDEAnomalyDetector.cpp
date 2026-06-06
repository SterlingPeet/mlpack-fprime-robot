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

KDEAnomalyDetector ::KDEAnomalyDetector(const char* const compName) :
    KDEAnomalyDetectorComponentBase(compName) {

    // Initialize mapping from telemetry IDs to dimensions.

    // Svc::SystemResources
    // Base ID of 0x10012000 comes from topology directly.
    dimMap[0x10012000] = 0; // MEMORY_TOTAL (u64)
    dimMap[0x10012001] = 1; // MEMORY_USED (u64)
    dimMap[0x10012002] = size_t(-1); // Intentionally ignored.
    dimMap[0x10012003] = size_t(-1);
    dimMap[0x10012004] = size_t(-1);
    dimMap[0x10012005] = 2; // CPU (f32)
    dimMap[0x10012006] = 3; // CPU1 (f32)
    dimMap[0x10012007] = 4; // CPU2 (f32)
    dimMap[0x10012008] = 5; // CPU3 (f32)
    dimMap[0x10012009] = 6; // CPU4 (f32)
    dimMap[0x1001200a] = 7; // CPU5 (f32)
    dimMap[0x1001200b] = 8; // CPU6 (f32)
    dimMap[0x1001200c] = 9; // CPU7 (f32)
    dimMap[0x1001200d] = 10; // CPU8 (f32)
    dimMap[0x1001200e] = 11; // CPU9 (f32)
    dimMap[0x1001200f] = 12; // CPU10 (f32)
    dimMap[0x10012010] = 13; // CPU11 (f32)
    dimMap[0x10012011] = 14; // CPU12 (f32)
    dimMap[0x10012012] = 15; // CPU13 (f32)
    dimMap[0x10012013] = 16; // CPU14 (f32)
    dimMap[0x10012014] = 17; // CPU15 (f32)

    dimTypes[0] = 0; // U64 (MEMORY_TOTAL)
    dimTypes[1] = 0; // U64 (MEMORY_USED)
    dimTypes[2] = 1; // F32 (CPU)
    dimTypes[3] = 1; // F32 (CPU1)
    dimTypes[4] = 1; // F32 (CPU2)
    dimTypes[5] = 1; // F32 (CPU3)
    dimTypes[6] = 1; // F32 (CPU4)
    dimTypes[7] = 1; // F32 (CPU5)
    dimTypes[8] = 1; // F32 (CPU6)
    dimTypes[9] = 1; // F32 (CPU7)
    dimTypes[10] = 1; // F32 (CPU8)
    dimTypes[11] = 1; // F32 (CPU9)
    dimTypes[12] = 1; // F32 (CPU10)
    dimTypes[13] = 1; // F32 (CPU11)
    dimTypes[14] = 1; // F32 (CPU12)
    dimTypes[15] = 1; // F32 (CPU13)
    dimTypes[16] = 1; // F32 (CPU14)
    dimTypes[17] = 1; // F32 (CPU15)

    tlmCache.resize(this->numDims);
}

KDEAnomalyDetector ::~KDEAnomalyDetector() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void KDEAnomalyDetector ::run_handler(FwIndexType portNum, U32 context) {
    // TODO: run anomaly detector
}

void KDEAnomalyDetector ::tlmIn_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    // Process an input telemetry message.
    if (dimMap.count(id) == 0)
    {
        std::ostringstream oss;
        oss << "id 0x" << std::hex << id << ", time " << std::dec
            << timeTag.getSeconds() << "." << timeTag.getUSeconds();
        Fw::LogStringArg logOut(oss.str().c_str());
        this->log_ACTIVITY_HI_InvalidTelemetryReceivedEvent(logOut);
        return; // Ignore unknown telemetry message.
    }

    size_t targetDim = dimMap[id];
    if (targetDim == size_t(-1))
    {
        return; // This event intentionally ignored.
    }

    size_t targetType = dimTypes[targetDim];
    double targetValue = 0.0;
    if (targetType == 0) // U64
    {
        U64 v;
        val.deserializeTo(v);
        targetValue = (double) v;
    }
    else if (targetType == 1) // F32
    {
        F32 v;
        val.deserializeTo(v);
        targetValue = (double) v;
    }
    else
    {
        Fw::LogStringArg logOut("invalid target dimension type (must be 0/1); "
            "see KDEAnomalyDetector.hpp");
        this->log_ACTIVITY_HI_InvalidTelemetryReceivedEvent(logOut);
        return;
    }

    tlmCache[targetDim][timeTag] = targetValue;

    // Drop any elements that are not part of the window.
    Fw::ParamValid isValid;
    U64 maxNumSamples = this->paramGet_MAX_NUM_SAMPLES(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT)
    {
        maxNumSamples = 10000;
    }

    Fw::Time lastTime = (--this->tlmCache[targetDim].end())->first;
    while (this->tlmCache[targetDim].size() > 0 &&
        Fw::Time::sub(lastTime,
                      this->tlmCache[targetDim].begin()->first).getSeconds() >
        maxNumSamples)
    {
        // Remove the first element; it is too far in the past to use.
        this->tlmCache[targetDim].erase(this->tlmCache[targetDim].begin());
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void KDEAnomalyDetector ::RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    // First assemble the dataset given historical data.
    // We must first find the most recent minimum time for all telemetry
    // dimensions; this will be the minimum cutoff for building the dataset.
    // We also need the most recent time across all telemetry.
    bool firstMaxMinTime = true;
    bool firstMaxTime = true;
    Fw::Time maxMinTime;
    Fw::Time maxTime;
    for (size_t i = 0; i < this->numDims; ++i)
    {
        const Fw::Time& t = this->tlmCache[i].begin()->first;
        if (firstMaxMinTime)
        {
            maxMinTime = t;
            firstMaxMinTime = false;
        }
        else if (t > maxMinTime)
        {
            maxMinTime = t;
        }

        const Fw::Time& t2 = (--this->tlmCache[i].end())->first;
        if (firstMaxTime)
        {
            maxTime = t2;
            firstMaxTime = false;
        }
        else if (t2 > maxTime)
        {
            maxTime = t2;
        }
    }

    if (maxTime <= maxMinTime)
    {
      // TODO: log error
      this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
      return;
    }

    // Now determine the size of our dataset.
    std::cout << "now compute numSeconds\n";
    U32 numSeconds = Fw::Time::sub(maxTime, maxMinTime).getSeconds();
    std::cout << "it is " << numSeconds << "\n";

    Fw::ParamValid isValid;
    U64 maxNumSamples = this->paramGet_MAX_NUM_SAMPLES(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT)
    {
        maxNumSamples = 10000;
    }

    U64 leafSize = this->paramGet_LEAF_SIZE(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT)
    {
        leafSize = 100;
    }

    F32 kernelBandwidth = this->paramGet_KERNEL_BW(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT)
    {
        kernelBandwidth = 1.0f;
    }

    F32 zeroDimNoise = this->paramGet_ZERO_DIM_NOISE(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT)
    {
        zeroDimNoise = 0.001f;
    }

    if (numSeconds > maxNumSamples)
    {
        numSeconds = maxNumSamples;
        maxMinTime = Fw::Time(maxTime.getTimeBase(),
                              maxTime.getSeconds() - maxNumSamples,
                              0);
    }

    // Now construct the dataset by iterating over each telemetry channel.
    arma::wall_clock c;
    c.tic();
    arma::mat dataset(this->numDims, numSeconds);
    Fw::Time currentTime = maxMinTime;
    std::array<std::map<Fw::Time, double>::const_iterator,
               this->numDims> tlmIters;
    for (size_t i = 0; i < this->numDims; ++i)
    {
        tlmIters[i] = this->tlmCache[i].begin();
    }

    // We construct the telemetry value for each channel as the
    // (non-interpolated) most recent value seen at each given point in time.
    for (size_t c = 0; c < numSeconds; ++c)
    {
        for (size_t r = 0; r < this->numDims; ++r)
        {
            // For each telemetry dimension, make sure we are looking at the
            // most recent observation *before* `currentTime`.
            while (tlmIters[r] != this->tlmCache[r].end() &&
                   tlmIters[r]->first < currentTime)
            {
                ++tlmIters[r];
            }

            // After the loop, we have walked one sample past where we want to
            // be.
            if (tlmIters[r] != this->tlmCache[r].begin())
            {
                --tlmIters[r];
            }

            dataset(r, c) = tlmIters[r]->second;
        }

        // Increment the time we are looking for by one second.
        currentTime += 1.0f;
    }

    // Normalize the dataset such that every dimension is zero-mean and unit
    // variance.  If the values in a dimension are all the same, we use the
    // parameter given for how much noise to add (this keeps our density
    // estimate from collapsing).
    this->dimMeans = arma::mean(dataset, 1);
    dataset -= arma::repmat(this->dimMeans, 1, dataset.n_cols);

    this->dimStddevs = arma::stddev(dataset, 0, 1);
    for (size_t d = 0; d < this->numDims; ++d)
    {
        if (this->dimStddevs[d] == 0.0)
        {
            dataset.row(d) += zeroDimNoise *
                arma::randn<arma::rowvec>(dataset.n_cols);
            this->dimStddevs[d] = 1.0;
        }
        else
        {
            dataset.row(d) /= this->dimStddevs[d];
        }
    }
    const F32 datasetBuildTime = (F32) c.toc();

    c.tic();
    this->kde.Train(std::move(dataset));
    const F32 treeBuildTime = (F32) c.toc();

    ++this->numResets;
    this->tlmWrite_RESET_COUNTER(this->numResets);
    this->tlmWrite_DATASET_BUILD_TIME(datasetBuildTime);
    this->tlmWrite_TREE_BUILD_TIME(treeBuildTime);
    this->tlmWrite_MODEL_DIMENSIONS(
        this->kde.ReferenceTree()->Dataset().n_rows);
    this->tlmWrite_MODEL_POINTS(this->kde.ReferenceTree()->Dataset().n_cols);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void KDEAnomalyDetector ::REPORT_DENSITY_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // Ensure that a model is trained.
    if (!this->kde.IsTrained())
    {
        // TODO: log error... no tree!
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    // Construct the most recent values of the telemetry.
    arma::vec point(this->numDims);
    for (size_t d = 0; d < this->numDims; ++d)
    {
        if (this->tlmCache[d].size() == 0)
        {
            // TODO: log error... no data!
            this->cmdResponse_out(opCode, cmdSeq,
                Fw::CmdResponse::EXECUTION_ERROR);
            return;
        }

        point[d] = (--this->tlmCache[d].end())->second;
    }

    // Normalize the point.
    point -= this->dimMeans;
    point /= this->dimStddevs;

    // Get the result.
    arma::vec estimate;
    this->kde.Evaluate(point, estimate);
    if (estimate.n_elem != 1)
    {
        // TODO: log error... invalid prediction size!
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    this->log_ACTIVITY_HI_ReportDensityEvent((F64) estimate[0]);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
