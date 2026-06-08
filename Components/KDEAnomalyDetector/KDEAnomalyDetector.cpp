// ======================================================================
// \title  KDEAnomalyDetector.cpp
// \author ryan
// \brief  cpp file for KDEAnomalyDetector component implementation class
// ======================================================================

#include "Components/KDEAnomalyDetector/KDEAnomalyDetector.hpp"
#include "Fw/Types/OnEnumAc.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

KDEAnomalyDetector ::KDEAnomalyDetector(const char* const compName) : KDEAnomalyDetectorComponentBase(compName) {
    // Initialize mapping from telemetry IDs to dimensions.

    // Svc::SystemResources
    // Base ID of 0x10012000 comes from topology directly.
    dimMap[0x10012000] = size_t(-1);  // MEMORY_TOTAL (u64)
    dimMap[0x10012001] = size_t(-1);  // MEMORY_USED (u64)
    dimMap[0x10012002] = size_t(-1);  // Intentionally ignored.
    dimMap[0x10012003] = size_t(-1);
    dimMap[0x10012004] = size_t(-1);
    dimMap[0x10012005] = 0;   // CPU (f32)
    dimMap[0x10012006] = 1;   // CPU1 (f32)
    dimMap[0x10012007] = 2;   // CPU2 (f32)
    dimMap[0x10012008] = 3;   // CPU3 (f32)
    dimMap[0x10012009] = 4;   // CPU4 (f32)
    dimMap[0x1001200a] = 5;   // CPU5 (f32)
    dimMap[0x1001200b] = 6;   // CPU6 (f32)
    dimMap[0x1001200c] = 7;   // CPU7 (f32)
    dimMap[0x1001200d] = 8;   // CPU8 (f32)
    dimMap[0x1001200e] = 9;   // CPU9 (f32)
    dimMap[0x1001200f] = 10;  // CPU10 (f32)
    dimMap[0x10012010] = 11;  // CPU11 (f32)
    dimMap[0x10012011] = 12;  // CPU12 (f32)
    dimMap[0x10012012] = 13;  // CPU13 (f32)
    dimMap[0x10012013] = 14;  // CPU14 (f32)
    dimMap[0x10012014] = 15;  // CPU15 (f32)

    // ROMI::RomiHWDriver
    // Base ID from 0x10007000 comes from topology directly.
    dimMap[0x10007000] = size_t(-1);  // I2C address of controller (ignored).
    dimMap[0x10007001] = 16;          // BatteryVoltage (F32)
    dimMap[0x10007002] = size_t(-1);  // Analog sensors (ignored).

    // ROMI::MotorCntrlManager
    // Base ID from 0x1000a000 comes from topology directly.
    dimMap[0x1000a000] = 17;  // LeftOdometry (I32)
    dimMap[0x1000a001] = 18;  // RightOdometry (I32)
    dimMap[0x1000a002] = 19;  // MotorsEnabled (Fw.On)
    dimMap[0x1000a003] = 20;  // LeftDelta (I16)
    dimMap[0x1000a004] = 21;  // RightDelta (I16)
    dimMap[0x1000a005] = 22;  // LeftVelocity (F32)
    dimMap[0x1000a006] = 23;  // RightVelocity (F32)
    dimMap[0x1000a007] = 24;  // LeftSpeed (I16)
    dimMap[0x1000a008] = 25;  // RightSpeed (I16)

    // ROMI::RomiIMU
    // Base ID from 0x1000b000 comes from topology directly.
    dimMap[0x1000b000] = 26;  // AccelX (F32)
    dimMap[0x1000b001] = 27;  // AccelY (F32)
    dimMap[0x1000b002] = 28;  // AccelZ (F32)
    dimMap[0x1000b003] = 29;  // GyroX (F32)
    dimMap[0x1000b004] = 30;  // GyroY (F32)
    dimMap[0x1000b005] = 31;  // GyroZ (F32)
    dimMap[0x1000b006] = 32;  // ImuTemp (F32)

    dimTypes[0] = 1;   // F32 (CPU)
    dimTypes[1] = 1;   // F32 (CPU1)
    dimTypes[2] = 1;   // F32 (CPU2)
    dimTypes[3] = 1;   // F32 (CPU3)
    dimTypes[4] = 1;   // F32 (CPU4)
    dimTypes[5] = 1;   // F32 (CPU5)
    dimTypes[6] = 1;   // F32 (CPU6)
    dimTypes[7] = 1;   // F32 (CPU7)
    dimTypes[8] = 1;   // F32 (CPU8)
    dimTypes[9] = 1;   // F32 (CPU9)
    dimTypes[10] = 1;  // F32 (CPU10)
    dimTypes[11] = 1;  // F32 (CPU11)
    dimTypes[12] = 1;  // F32 (CPU12)
    dimTypes[13] = 1;  // F32 (CPU13)
    dimTypes[14] = 1;  // F32 (CPU14)
    dimTypes[15] = 1;  // F32 (CPU15)
    dimTypes[16] = 1;  // F32 (BatteryVoltage)
    dimTypes[17] = 2;  // I32 (LeftOdometry)
    dimTypes[18] = 2;  // I32 (RightOdometry)
    dimTypes[19] = 4;  // Fw::On (MotorsEnabled)
    dimTypes[20] = 3;  // I16 (LeftDelta)
    dimTypes[21] = 3;  // I16 (RightDelta)
    dimTypes[22] = 1;  // F32 (LeftVelocity)
    dimTypes[23] = 1;  // F32 (RightVelocity)
    dimTypes[24] = 3;  // I16 (LeftSpeed)
    dimTypes[25] = 3;  // I16 (RightSpeed)
    dimTypes[26] = 1;  // F32 (AccelX)
    dimTypes[27] = 1;  // F32 (AccelY)
    dimTypes[28] = 1;  // F32 (AccelZ)
    dimTypes[29] = 1;  // F32 (GyroX)
    dimTypes[30] = 1;  // F32 (GyroY)
    dimTypes[31] = 1;  // F32 (GyroZ)
    dimTypes[32] = 1;  // F32 (ImuTemp)

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
    if (dimMap.count(id) == 0) {
        std::ostringstream oss;
        oss << "id 0x" << std::hex << id << ", time " << std::dec << timeTag.getSeconds() << "."
            << timeTag.getUSeconds();
        Fw::LogStringArg logOut(oss.str().c_str());
        this->log_ACTIVITY_HI_InvalidTelemetryReceivedEvent(logOut);
        return;  // Ignore unknown telemetry message.
    }

    size_t targetDim = dimMap[id];
    if (targetDim == size_t(-1)) {
        return;  // This event intentionally ignored.
    }

    size_t targetType = dimTypes[targetDim];
    double targetValue = 0.0;
    if (targetType == 0)  // U64
    {
        U64 v;
        val.deserializeTo(v);
        targetValue = (double)v;
    } else if (targetType == 1)  // F32
    {
        F32 v;
        val.deserializeTo(v);
        targetValue = (double)v;
    } else if (targetType == 2)  // I32
    {
        I32 v;
        val.deserializeTo(v);
        targetValue = (double)v;
    } else if (targetType == 3)  // I16
    {
        I16 v;
        val.deserializeTo(v);
        targetValue = (double)v;
    } else if (targetType == 4)  // Fw::On
    {
        Fw::On v;
        val.deserializeTo(v);
        targetValue = (v == Fw::On::ON) ? 1.0 : 0.0;
    } else {
        Fw::LogStringArg logOut(
            "invalid target dimension type (must be 0/1); "
            "see KDEAnomalyDetector.hpp");
        this->log_ACTIVITY_HI_InvalidTelemetryReceivedEvent(logOut);
        return;
    }

    tlmCache[targetDim][timeTag] = targetValue;

    // Drop any elements that are not part of the window.
    Fw::ParamValid isValid;
    U64 maxNumSamples = this->paramGet_MAX_NUM_SAMPLES(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT) {
        maxNumSamples = 10000;
    }

    Fw::Time lastTime = (--this->tlmCache[targetDim].end())->first;
    while (this->tlmCache[targetDim].size() > 0 &&
           Fw::Time::sub(lastTime, this->tlmCache[targetDim].begin()->first).getSeconds() > maxNumSamples) {
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
    for (size_t i = 0; i < this->numDims; ++i) {
        if (this->tlmCache[i].size() == 0) {
            // Skip empty dimensions.
            continue;
        }

        const Fw::Time& t = this->tlmCache[i].begin()->first;

        if (firstMaxMinTime) {
            maxMinTime = t;
            firstMaxMinTime = false;
        } else if (t > maxMinTime) {
            maxMinTime = t;
        }

        const Fw::Time& t2 = (--this->tlmCache[i].end())->first;
        if (firstMaxTime) {
            maxTime = t2;
            firstMaxTime = false;
        } else if (t2 > maxTime) {
            maxTime = t2;
        }
    }

    if (maxTime <= maxMinTime) {
        // TODO: log error
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    // Now determine the size of our dataset.
    U32 numSeconds = Fw::Time::sub(maxTime, maxMinTime).getSeconds();

    Fw::ParamValid isValid;
    U64 maxNumSamples = this->paramGet_MAX_NUM_SAMPLES(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT) {
        maxNumSamples = 10000;
    }

    U64 leafSize = this->paramGet_LEAF_SIZE(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT) {
        leafSize = 100;
    }

    F32 kernelBandwidth = this->paramGet_KERNEL_BW(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT) {
        kernelBandwidth = 1.0f;
    }

    F32 zeroDimNoise = this->paramGet_ZERO_DIM_NOISE(isValid);
    if (isValid == Fw::ParamValid::INVALID || isValid == Fw::ParamValid::UNINIT) {
        zeroDimNoise = 0.001f;
    }

    if (numSeconds > maxNumSamples) {
        numSeconds = maxNumSamples;
        maxMinTime = Fw::Time(maxTime.getTimeBase(), maxTime.getSeconds() - maxNumSamples, 0);
    }

    // Now construct the dataset by iterating over each telemetry channel.
    arma::wall_clock c;
    c.tic();
    arma::mat dataset(this->numDims, numSeconds, arma::fill::none);
    Fw::Time currentTime = maxMinTime;
    std::array<std::map<Fw::Time, double>::const_iterator, this->numDims> tlmIters;
    for (size_t i = 0; i < this->numDims; ++i) {
        tlmIters[i] = this->tlmCache[i].begin();
    }

    // We construct the telemetry value for each channel as the
    // (non-interpolated) most recent value seen at each given point in time.
    for (size_t c = 0; c < numSeconds; ++c) {
        for (size_t r = 0; r < this->numDims; ++r) {
            if (this->tlmCache[r].size() == 0) {
                // If we have no telemetry values, consider it to be zero.
                dataset(r, c) = 0.0;
                continue;
            }

            // For each telemetry dimension, make sure we are looking at the
            // most recent observation *before* `currentTime`.
            while (tlmIters[r] != this->tlmCache[r].end() && tlmIters[r]->first < currentTime) {
                ++tlmIters[r];
            }

            // After the loop, we have walked one sample past where we want to
            // be.
            if (tlmIters[r] != this->tlmCache[r].begin()) {
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
    for (size_t d = 0; d < this->numDims; ++d) {
        if (this->dimStddevs[d] == 0.0) {
            dataset.row(d) += zeroDimNoise * arma::randn<arma::rowvec>(dataset.n_cols);
            this->dimStddevs[d] = 1.0;
        } else {
            dataset.row(d) /= this->dimStddevs[d];
        }
    }
    const F32 datasetBuildTime = (F32)c.toc();

    c.tic();
    this->kde.Train(std::move(dataset));
    const F32 treeBuildTime = (F32)c.toc();

    ++this->numResets;
    this->tlmWrite_RESET_COUNTER(this->numResets);
    this->tlmWrite_DATASET_BUILD_TIME(datasetBuildTime);
    this->tlmWrite_TREE_BUILD_TIME(treeBuildTime);
    this->tlmWrite_MODEL_DIMENSIONS(this->kde.ReferenceTree()->Dataset().n_rows);
    this->tlmWrite_MODEL_POINTS(this->kde.ReferenceTree()->Dataset().n_cols);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void KDEAnomalyDetector ::REPORT_DENSITY_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // Ensure that a model is trained.
    if (!this->kde.IsTrained()) {
        // TODO: log error... no tree!
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    // Construct the most recent values of the telemetry.
    arma::vec point(this->numDims);
    for (size_t d = 0; d < this->numDims; ++d) {
        if (this->tlmCache[d].size() == 0) {
            point[d] = 0.0;
        } else {
            point[d] = (--this->tlmCache[d].end())->second;
        }
    }

    // Normalize the point.
    point -= this->dimMeans;
    point /= this->dimStddevs;

    // Get the result.
    arma::vec estimate;
    this->kde.Evaluate(point, estimate);
    if (estimate.n_elem != 1) {
        // TODO: log error... invalid prediction size!
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    this->log_ACTIVITY_HI_ReportDensityEvent((F64)estimate[0]);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
