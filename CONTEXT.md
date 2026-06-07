# MarsRobot — F Prime Implementation Context

MarsRobot is an F Prime port of the cFS-based MoonRobot demo.  Both projects
drive a Pololu Romi 32U4 robot over I2C from a Raspberry Pi, but MarsRobot
replaces the monolithic cFS `romimot` app with a layered F Prime component
hierarchy and replaces the simple proportional controller with a full PID
control stack.

---

## Hardware

| Item | Detail |
|------|--------|
| Robot | Pololu Romi 32U4 |
| Host | Raspberry Pi (any model with I2C-1) |
| I2C bus | `/dev/i2c-1`, address `0x14` (20 decimal) |
| Arduino firmware | `arduino_code/RomiRPiRemoteControl/RomiRPiRemoteControl.ino` (PololuRPiSlave) |

---

## I2C Protocol

The Romi 32U4 runs PololuRPiSlave which exposes a shared-memory buffer over
I2C.  The protocol is register-addressed SMBUS-style, but with an important
timing constraint: the 32U4 needs ~100 µs between receiving the register-address
byte and the host issuing a read.

**The Linux `I2C_RDWR` ioctl (atomic repeated-start) does not insert this gap
and produces stale or garbled data on the Romi.**  `RomiI2cDriver` performs a
discrete `write()` → `usleep(100 µs)` → `read()` sequence instead.

### Wire Format (PololuRPiSlave shared buffer)

All multi-byte integers are **little-endian** (AVR/ARM native).  The buffer is
accessed by register address (= byte offset).

| Offset | Bytes | Field | R/W | `RomiRegister` enum |
|--------|-------|-------|-----|---------------------|
| 0 | 2 | `leftEncoder` I16 | R | `ENCODERS = 0` |
| 2 | 2 | `rightEncoder` I16 | R | — |
| 4 | 2 | `batteryMillivolts` U16 | R | `BATTERY_VOLTAGE = 4` |
| 6 | 12 | `analog[6]` U16 each | R | `ADC_CHANNELS = 6` |
| 18 | 1 | `buttonA` bool | R | `BUTTON_A = 18` |
| 19 | 1 | `buttonB` bool | R | `BUTTON_B = 19` |
| 20 | 1 | `buttonC` bool | R | `BUTTON_C = 20` |
| 21 | 1 | `yellowLed` bool | **W** | `YELLOW_LED = 21` |
| 22 | 1 | `greenLed` bool | **W** | `GREEN_LED = 22` |
| 23 | 1 | `redLed` bool | **W** | `RED_LED = 23` |
| 24 | 2 | `leftMotor` I16 | **W** | `MOTOR_CMDS = 24` |
| 26 | 2 | `rightMotor` I16 | **W** | — |
| 28 | 1 | `playNotes` bool | **W** | `PLAY_NOTES = 27`* |
| 29 | 14 | `notes[14]` char | **W** | `NOTES = 28`* |

\* Off-by-one between `RomiTypes.fpp` enum value and byte offset — verify
   before use.

> **Note:** the cFS `romimot_hw.c` constants (`romiCmdMotor = 6`,
> `romiCmdEncoder = 39`) are **wrong** for this `.ino` sketch.  The
> `RomiTypes.fpp` enum values listed above are correct.

### Per-cycle I2C Transactions (RomiHWDriver schedIn)

`RomiHWDriver` caches all pending LED and motor writes between `schedIn` calls.
Each `schedIn` issues exactly two transactions:

1. **Write** (8 bytes, via `i2cWrite` → `RomiI2cDriver.write`):
   `[21, yellow, green, red, L_lo, L_hi, R_lo, R_hi]`
   — covers all writeable registers in one contiguous block.

2. **WriteRead** (via `i2cWriteRead` → `RomiI2cDriver.writeRead`):
   write `[0]`, delay 100 µs, read 28 bytes — full struct snapshot.

---

## Component Hierarchy

```
RomiI2cDriver
    └── RomiHWDriver
            └── MotorCntrlManager
                    └── PidCtrlManager
                            └── PathSegmentManager
```

### RomiI2cDriver  `Components/RomiI2cDriver/`
**Status: implemented**

Drop-in replacement for `Drv.LinuxI2cDriver` on Romi I2C buses.  Exposes
the same `Drv.I2c` write/read and `Drv.I2cWriteRead` port surface, but
`writeRead_handler` uses `write → usleep → read` instead of `I2C_RDWR`.

Key file: `config/RomiCfg.hpp` — holds `I2C_READ_DELAY_US` and `I2C_DEVICE`.

Call `romiI2cDriver.open(ROMI::I2C_DEVICE)` from `configureTopology()`.

### RomiHWDriver  `Components/RomiHWDriver/`
**Status: stub — redesign pending**

Hardware abstraction layer.  Translates typed F Prime port calls into I2C
register reads and writes.  Responsibilities:

- Cache motor and LED state; flush to hardware in one write per `schedIn`.
- Read full 28-byte telemetry struct and parse little-endian fields manually
  (FPP-generated `RomiTelemetry` serialization format ≠ wire format).
- Emit encoder values on `sendEncoders` output port each cycle.
- Poll button state; push changes via `sendButton[n]` (≤20 ms at 50 Hz).
- Emit `BatteryVoltage` telemetry channel (always).
- Configurable I2C address via `I2C_ADDRESS` parameter (default 0x14).

FPP changes needed from current stub:
- Replace single `i2c: Drv.I2cWriteRead` with `i2cWrite: Drv.I2c` +
  `i2cWriteRead: Drv.I2cWriteRead` output ports.
- Remove `I2cCommand` internal port (replaced by cached-write model).
- Replace heap-allocated buffers with static member arrays.

### MotorCntrlManager  `Components/MotorCntrlManager/`
**Status: not started**

Integrates encoder deltas into absolute odometry (I32 each wheel).  Manages
motor enable/disable.  Receives setpoints from `PidCtrlManager` and forwards
computed motor power to `RomiHWDriver.setMotors`.

### PidCtrlManager  `Components/PidCtrlManager/`
**Status: not started**

Full PID controller with configurable Kp/Ki/Kd parameters and output
clamping (±200, matching Romi motor range).  Replaces the simple P-controller
from the cFS demo.

### PathSegmentManager  `Components/PathSegmentManager/`
**Status: not started**

High-level path primitives: STRAIGHT, ARC, STOP.  Translates path commands
into per-wheel odometry setpoints for `PidCtrlManager`.

---

## Rate Groups

Target: 50 Hz base timer (20 ms period).

| Rate Group | Divisor | Hz | Connected to |
|------------|---------|-----|-------------|
| rateGroup4 | 1 | 50 | romiHWDriver, motorCntrlMgr, pidCtrlMgr |
| rateGroup1 | 50 | 1 | telemetry, system resources, anomaly detector |
| rateGroup2 | 100 | 0.5 | command sequencer |
| rateGroup3 | 200 | 0.25 | health, comms, data products |

Timer call in `MarsRobotTopology.cpp`:
```cpp
// 20 ms period → 50 Hz base rate
timer.startTimer(Fw::TimeInterval(0, 20'000'000));
```

Rate group divisors in `MarsRobotTopology.cpp`:
```cpp
Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {50, 0}, {100, 0}, {200, 0}}};
```

---

## Key Design Notes

1. **Wire format vs FPP types** — `RomiTelemetry` in `RomiTypes.fpp` documents
   the logical struct but cannot be used for direct I2C deserialization.  Parse
   bytes manually in `schedIn_handler` using the offset table above.

2. **sendButton port semantics** — `sendButton: [3] Drv.GpioRead` on
   `RomiHWDriver` is polled (not interrupt-driven).  Button changes are detected
   by comparing the current read against `m_lastButtonState[3]`.  When the
   port is connected, `RomiHWDriver` calls `sendButton_out(n, state)` passing
   the cached state through the `ref Fw.Logic` parameter.  When disconnected,
   a `ButtonStateChange` event is emitted instead.  Polling latency ≤20 ms at
   50 Hz, which is within the 30 ms application requirement.

3. **I2C address** — the `I2C_ADDRESS` parameter on `RomiHWDriver` (default 20 /
   0x14) allows the address to be changed via ground command without recompiling.

4. **Stubbed drivers** — build with `-DFPRIME_USE_STUBBED_DRIVERS=ON` to compile
   and run on macOS.  `RomiI2cDriver` contains Linux-specific headers so a
   stub version may be needed for host builds.

---

## Implementation Checklist

- [x] `config/RomiCfg.hpp` — timing constants
- [x] `Components/RomiI2cDriver/` — custom I2C driver
- [ ] `Components/RomiHWDriver/` — redesign FPP ports, implement schedIn
- [ ] `Components/MotorCntrlManager/` — odometry + enable/disable
- [ ] `Components/PidCtrlManager/` — PID controller
- [ ] `Components/PathSegmentManager/` — path primitives
- [ ] `MarsRobot/Top/instances.fpp` — add rateGroup4 + new instances
- [ ] `MarsRobot/Top/topology.fpp` — wire new connections
- [ ] `MarsRobot/Top/MarsRobotTopology.cpp` — 50 Hz timer, romiI2cDriver.open()
