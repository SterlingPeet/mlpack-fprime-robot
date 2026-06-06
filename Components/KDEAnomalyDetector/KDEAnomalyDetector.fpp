module Components {
    @ Telemtry anomaly detector using mlpack's KDE (kernel density estimation) implementation.
    active component KDEAnomalyDetector {

        # Command for resetting the KDE model.
        async command RESET()

        # @ Example async command
        async command REPORT_DENSITY()

        # @ Count the number of times the model has been reset.
        telemetry ResetCounter: U64
        # @ The UNIX timestamp of the last time the model was reset.
        telemetry LastResetTimestamp: U64

        # @ Number of dimensions that the last model was trained on.
        telemetry MODEL_DIMENSIONS: U64
        # @ Number of points that the last model was trained on.
        telemetry MODEL_POINTS: U64

        # @ Event issued on model reset.
        event ResetEvent() severity activity high id 0 format "KDE model reset"

        # @ Event issued when the current density is asked for.
        event ReportDensityEvent(density: F64) severity activity high id 1 format "KDE model current density: {}"

        # @ Event issued when telemetry is received.
        event TelemetryReceivedEvent(info: string size 1024) severity activity high id 2 format "Telemetry received: {}"

        # @ The rate group scheduler input
        sync input port run: Svc.Sched

        # @ The input used when a telemetry event is sent.
        sync input port tlmIn: Fw.Tlm

        # @ Leaf size to use when building the tree.
        param LEAF_SIZE: U64 default 100
        # @ Maximum number of historical samples to use when building the tree.
        param MAX_NUM_SAMPLES: U64 default 10000

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
