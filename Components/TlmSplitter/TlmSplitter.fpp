module Components {
    @ Shim splitter for routing telemetry to multiple places.
    passive component TlmSplitter {

      @ Telemetry input
      sync input port TlmRecv: Fw.Tlm

      @ Telemetry output A
      output port TlmOutA: Fw.Tlm

      @ Telemetry output B
      output port TlmOutB: Fw.Tlm

    }
}
