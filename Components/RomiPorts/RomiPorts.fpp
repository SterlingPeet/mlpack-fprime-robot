module ROMI {

  @ Port to command the speed of a pair of motors
  port MotorPairSpeed (
    speed: MotorCommand @< Speed of the left and right motors
  )

  @ Port to report the encoder values of a pair of motors
  port MotorPairEncoders (
    encoders: MotorEncoders @< Encoder values for the left and right motors
  )

  @ Port to play notes
  port PlayNotes($notes: string size 14)
}
