
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
sensor_actor_interfaces
holger_fc
holger_blmc_driver
holger_blmc
rc_dsl_driver
rc_dsl
scl_gps
scl_voltage
maxsonar_driver
maxsonar
bmp085_driver
bmp085
chr_6dm
geometry
i2c serial logger opcd_params threads ltime lmath scl
glib-2.0
util network_messages
scl_messages
filters
meschach zmq yaml-cpp rt pthread m protobuf-c)

INSTALL_TARGETS(/bin core)
