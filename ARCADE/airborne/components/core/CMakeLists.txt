
cmake_minimum_required(VERSION 2.8)

ADD_SUBDIRECTORY(hardware)
ADD_SUBDIRECTORY(control)
ADD_SUBDIRECTORY(geometry)
ADD_SUBDIRECTORY(platform)
ADD_SUBDIRECTORY(model)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(filters)
ADD_SUBDIRECTORY(command)


ADD_EXECUTABLE(core main.c)

TARGET_LINK_LIBRARIES(core
daemon
command
control
model
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
geometry
i2c serial logger opcd_params threads ltime scl
glib-2.0
util network_messages
scl_messages
filters
cvxgen
hardware_util
meschach zmq yaml-cpp rt pthread m protobuf-c)

INSTALL_TARGETS(/bin core)
