#include <Wire.h>
#include <Romi32U4.h>

/* Romi 32U4 I2C slave firmware for MarsRobot.
 *
 * This replaces the Pololu PololuRPiSlave library with a direct
 * implementation on the stock Arduino Wire library.  We own both ends of
 * this bus (the Romi 32U4 and the Raspberry Pi running F Prime), so we
 * define a small register-addressed protocol and deliberately avoid any
 * I2C feature that the Pi's BCM I2C peripheral handles poorly.
 *
 * Protocol (no repeated-start -- the BCM repeated-start is broken):
 *
 *   WRITE transaction:  [reg, data0, data1, ...]  STOP
 *       Stores the data bytes into the shared buffer starting at offset
 *       `reg` (auto-incrementing).  This is master -> Romi command data.
 *       Writes to read-only offsets (< WRITABLE_START) are ignored so
 *       sensor data stays authoritative.
 *
 *   READ (two separate bus transactions, no repeated-start):
 *       1) [reg]            STOP   -- latches the read offset
 *       2) <read N bytes>   STOP   -- streams from `reg` onward
 *       The master inserts a short delay between (1) and (2); see
 *       I2C_READ_DELAY_US on the Pi side.  A real STOP separates the two
 *       transactions, and there is no multi-master contention, so the
 *       latched offset is still valid when the read arrives.
 *
 * Register map (little-endian; byte offset == register address):
 *
 *   off  size  field               dir
 *   0    2     leftEncoder   i16   R
 *   2    2     rightEncoder  i16   R
 *   4    2     batteryMv     u16   R
 *   6    12    analog[6]     u16   R
 *   18   1     buttonA       u8    R
 *   19   1     buttonB       u8    R
 *   20   1     buttonC       u8    R
 *   21   1     yellowLed     u8    W
 *   22   1     greenLed      u8    W
 *   23   1     redLed        u8    W
 *   24   2     leftMotor     i16   W
 *   26   2     rightMotor    i16   W
 *   28   1     playNotes     u8    W
 *   29   14    notes[14]     char  W
 *   ----------------------------------- total 43 bytes
 */

static const uint8_t I2C_ADDRESS = 20;  // 0x14

// Shared register file.  Touched by both loop() and the I2C ISRs, so
// loop() brackets every multi-byte update with noInterrupts()/interrupts()
// to guarantee the onRequest ISR never streams a torn value.
struct __attribute__((packed)) Data
{
    int16_t  leftEncoder;        // 0
    int16_t  rightEncoder;       // 2
    uint16_t batteryMillivolts;  // 4
    uint16_t analog[6];          // 6
    uint8_t  buttonA;            // 18
    uint8_t  buttonB;            // 19
    uint8_t  buttonC;            // 20
    uint8_t  yellow;             // 21
    uint8_t  green;              // 22
    uint8_t  red;                // 23
    int16_t  leftMotor;          // 24
    int16_t  rightMotor;         // 26
    uint8_t  playNotes;          // 28
    char     notes[14];          // 29
};                               // 43 bytes total

union Buffer
{
    Data    data;
    uint8_t bytes[sizeof(Data)];
};

static volatile Buffer  buffer;
static volatile uint8_t i2cReg = 0;  // latched register pointer

// First writable register.  Master writes below this offset are dropped.
static const uint8_t WRITABLE_START = 21;

PololuBuzzer     buzzer;
Romi32U4Motors   motors;
Romi32U4ButtonA  buttonA;
Romi32U4ButtonB  buttonB;
Romi32U4ButtonC  buttonC;
Romi32U4Encoders encoders;

// ---------------------------------------------------------------------------
// I2C ISR callbacks
// ---------------------------------------------------------------------------

// Master wrote bytes to us.  The first byte is always the register offset;
// any remaining bytes are a write payload stored from that offset onward.
void onReceiveHandler(int numBytes)
{
    if (numBytes <= 0)
    {
        return;
    }

    i2cReg = (uint8_t)Wire.read();
    numBytes--;

    uint8_t idx = i2cReg;
    while (numBytes-- > 0 && Wire.available())
    {
        uint8_t b = (uint8_t)Wire.read();
        if (idx >= WRITABLE_START && idx < sizeof(buffer.bytes))
        {
            buffer.bytes[idx] = b;
        }
        idx++;
    }
}

// Master is reading from us.  Stream from the latched register offset,
// capped at the Wire TX buffer length (32 on AVR).  Our largest read is
// the 21-byte telemetry block (offsets 0..20), so this never truncates.
void onRequestHandler()
{
    uint8_t start = i2cReg;
    if (start >= sizeof(buffer.bytes))
    {
        return;
    }
    uint8_t len = sizeof(buffer.bytes) - start;
    if (len > BUFFER_LENGTH)
    {
        len = BUFFER_LENGTH;
    }
    Wire.write((const uint8_t*)&buffer.bytes[start], len);
}

void setup()
{
    Wire.begin(I2C_ADDRESS);  // join the I2C bus as a slave
    Wire.onReceive(onReceiveHandler);
    Wire.onRequest(onRequestHandler);

    // Startup chirp.
    buzzer.play("v10>>g16>>>c16");
}

void loop()
{
    // --- Snapshot master-written command fields ----------------------------
    uint8_t setYellow, setGreen, setRed, setPlay;
    int16_t setLeft, setRight;
    char    setNotes[15];

    noInterrupts();
    setYellow = buffer.data.yellow;
    setGreen  = buffer.data.green;
    setRed    = buffer.data.red;
    setLeft   = buffer.data.leftMotor;
    setRight  = buffer.data.rightMotor;
    setPlay   = buffer.data.playNotes;
    memcpy(setNotes, (const void*)buffer.data.notes, 14);
    interrupts();
    setNotes[14] = '\0';

    // --- Apply outputs -----------------------------------------------------
    ledYellow(setYellow);
    ledGreen(setGreen);
    ledRed(setRed);

    // Romi motor speeds are valid in [-300, 300]; ignore out-of-range
    // commands rather than clamping silently.
    if (setLeft >= -300 && setLeft <= 300 && setRight >= -300 && setRight <= 300)
    {
        motors.setSpeeds(setLeft, setRight);
    }

    // Play a tune once per rising edge of playNotes, then self-clear.
    static bool startedPlaying = false;
    if (setPlay && !startedPlaying)
    {
        buzzer.play(setNotes);
        startedPlaying = true;
    }
    else if (startedPlaying && !buzzer.isPlaying())
    {
        startedPlaying = false;
        noInterrupts();
        buffer.data.playNotes = 0;
        interrupts();
    }

    // --- Read sensors ------------------------------------------------------
    int16_t  le = encoders.getCountsLeft();
    int16_t  re = encoders.getCountsRight();
    uint16_t bv = readBatteryMillivolts();  // use readBatteryMillivoltsLV() on the LV model
    uint16_t a[6];
    for (uint8_t i = 0; i < 6; i++)
    {
        a[i] = analogRead(i);
    }
    uint8_t ba = buttonA.isPressed() ? 1 : 0;
    uint8_t bb = buttonB.isPressed() ? 1 : 0;
    uint8_t bc = buttonC.isPressed() ? 1 : 0;

    // --- Publish sensor fields (single critical section) -------------------
    noInterrupts();
    buffer.data.leftEncoder       = le;
    buffer.data.rightEncoder      = re;
    buffer.data.batteryMillivolts = bv;
    for (uint8_t i = 0; i < 6; i++)
    {
        buffer.data.analog[i] = a[i];
    }
    buffer.data.buttonA = ba;
    buffer.data.buttonB = bb;
    buffer.data.buttonC = bc;
    interrupts();
}
