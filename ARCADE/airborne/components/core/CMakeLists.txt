
cmake_minimum_required(VERSION 2.8)

ADD_SUBDIRECTORY(hardware)
ADD_SUBDIRECTORY(control)
ADD_SUBDIRECTORY(geometry)
ADD_SUBDIRECTORY(platform)
ADD_SUBDIRECTORY(estimators)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(filters)
ADD_SUBDIRECTORY(command)

ADD_EXECUTABLE(core motostate.c main.c calpub.c flight_detect.c)


TARGET_LINK_LIBRARIES(core
daemon
command
basic_control
speed_control
position_control
estimators
platform
holger_blmc
rc_dsl_reader
rc_dsl
scl_gps
scl_voltage
maxsonar_reader
maxsonar
i2cxl
i2cxl_reader
bma180
hmc5883
itg3200
mpu6050
ms5611
ms5611_reader
geometry
i2c serial logger opcd_params threads ltime scl
glib-2.0
util network_messages
scl_messages
filters
vec2
cvxgen
hardware_util
meschach zmq yaml-cpp rt pthread m protobuf-c)
INSTALL_TARGETS(/bin core)

