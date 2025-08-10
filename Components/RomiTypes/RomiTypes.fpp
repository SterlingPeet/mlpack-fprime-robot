module ROMI {

  @ Default I2C Address for the Romi
  constant ROMI_I2C_DEFAULT_ADDRESS = 20 @< Default I2C Address for the Romi

  @Enum representing the addresses for each register on the Romi
  enum RomiRegister {
    ENCODERS = 0 @< Left motor encoder value
    ENCODER_RIGHT = 2 @< Right motor encoder value
    BATTERY_VOLTAGE = 4 @< Battery voltage in millivolts
    ADC_CHANNELS = 6 @< ADC channel readings
    BUTTON_A = 18 @< Button A state
    BUTTON_B = 19 @< Button B state
    BUTTON_C = 20 @< Button C state
    YELLOW_LED = 21 @< Yellow LED state
    GREEN_LED = 22 @< Green LED state
    RED_LED = 23 @< Red LED state
    MOTOR_CMDS = 24 @< Left motor command value
    MOTOR_CMD_RIGHT = 26 @< Right motor command value
    PLAY_NOTES = 27 @< Play notes command
    NOTES = 28 @< Notes to play
  }

  @ Enum to describe the available buttons on the Romi
  enum RomiButton {
    A = 0 @< Button A
    B = 1 @< Button B
    C = 2 @< Button C
  }

  @ Enum to describe the Romi button state
  enum ButtonState {
    RELEASED @< Button is released
    PRESSED @< Button is pressed
  }

  @ Struct holding the Romi motor encoder values or deltas.
  struct MotorEncoders {
    left: I16 @< Left motor encoder value
    right: I16 @< Right motor encoder value
  }

  @ Struct holding the Romi motor command values.
  struct MotorCommand {
    left: I16 @< Left motor command value
    right: I16 @< Right motor command value
  }

  @ Struct for deserializing the Romi telemetry I2C format
  struct RomiTelemetry {
    encoders: MotorEncoders @< Motor encoder values
    batteryVoltage: U16 @< Battery voltage in millivolts
    adcChannels: [6] U16 @< ADC channel readings
    buttonA: ButtonState @< Button A state
    buttonB: ButtonState @< Button B state
    buttonC: ButtonState @< Button C state
    yellowLed: Fw.On @< Yellow LED state
    greenLed: Fw.On @< Green LED state
    redLed: Fw.On @< Red LED state
    motorCmd: MotorCommand @< Motor command values
  }

}
