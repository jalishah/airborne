# Core
## Purpose
This component is a real-time low-level flight controller using flight-relevant sensors and actuators.

## Directory Structure:
```
├── controllers: yaw/pitch/roll/navigation controllers
├── interfaces: interfaces for controlling the core
├── model: state estimation code
├── platform: platform-specific code (quadro/hexa-rotors)
├── sensor_actor: sensor and actuator code
└── util: utility modules
```

## Sensors:

- CHR-6DM attitude and heading reference system: euler angles, accelerometers, gyro values
- Ublox LEA-4GPS: global GPS position
- ultrasonic range finder: max. 7 meters above the ground
- barometric pressure sensor: meters above sea level
- voltage sensor: voltage divider on VBat connected to ADC7
- RPM sensor: indicates motor RPM (read from brushless motor controllers)

## Actuators:

- interface to holger flight control in heading-hold (aka axis lock) mode
- signal light (xenon flash)
- audio output via espeak/mp3 player
