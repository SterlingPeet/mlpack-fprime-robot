module Components {
    @ Telemetry anomaly detector using mlpack's KDE (kernel density estimation) implementation.
    active component KDEAnomalyDetector {

        # Command for resetting the KDE model.
        async command RESET()

        # @ Example async command
        async command REPORT_DENSITY()

        # @ Count the number of times the model has been reset.
        telemetry RESET_COUNTER: U64

        # @ Number of seconds taken to build the dataset for the kd-tree.
        telemetry DATASET_BUILD_TIME: F32
        # @ Number of seconds taken to build the tree.
        telemetry TREE_BUILD_TIME: F32
        # @ Number of dimensions that the last model was trained on.
        telemetry MODEL_DIMENSIONS: U64
        # @ Number of points that the last model was trained on.
        telemetry MODEL_POINTS: U64
        # @ Current density reading.  -1 if no model is trained.
        telemetry CURRENT_DENSITY: F64

        # @ Event issued on model reset.
        event ResetEvent() severity activity high id 0 format "KDE model reset"

        # @ Event issued when the current density is asked for.
        event ReportDensityEvent(density: F64) severity activity high id 1 format "KDE model current density: {}"

        # @ Event issued when invalid telemetry is received.
        event InvalidTelemetryReceivedEvent(info: string size 1024) severity activity high id 2 format "Invalid telemetry received: {}"

        # @ The rate group scheduler input
        sync input port run: Svc.Sched

        # @ The input used when a telemetry event is sent.
        sync input port tlmIn: Fw.Tlm

        # @ Output port to send notes to the RomiHWDriver.
        output port playNotes: ROMI.PlayNotes

        # @ Leaf size to use when building the tree.
        param LEAF_SIZE: U64 default 100
        # @ Maximum number of historical samples to use when building the tree.
        param MAX_NUM_SAMPLES: U64 default 10000
        # @ Bandwidth of Gaussian kernel to use.
        param KERNEL_BW: F32 default 1.0
        # @ Variance of noise to add to zero dimensions.
        param ZERO_DIM_NOISE: F32 default 0.001
        # @ Threshold for anomaly detection.
        param ANOMALY_THRESHOLD: F32 default 1e-6
        # @ Number of samples we need to detect an anomaly to really flag it.
        param ANOMALY_SAMPLES_BEFORE_ALARM: U64 default 5

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Enables command handling
        import Fw.Command

        @ Enables event handling
        import Fw.Event

        @ Enables telemetry channels handling
        import Fw.Channel

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}
