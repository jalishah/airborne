# component: core
## purpose
This component is a real-time low-level flight controller using flight-relevant sensors and actuators.
Sensor:

- attitude and heading reference system: euler angles, accelerometers, gyro values
- GPS: information about the global position
- ultrasonic distance: max. 7 meters above the ground
- barometric pressure: meters above sea level

Actuators:
- interface to holger flight control in heading-hold (aka axis lock) mode
- signal light (xenon flash)
