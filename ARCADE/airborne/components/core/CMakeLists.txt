
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
# system:
control
model
# sensor/actor:
platform
sensor_actor_interfaces
holger_fc
holger_blmc_driver
holger_blmc
rc_dsl_driver
rc_dsl
scl_gps
maxsonar_driver
maxsonar
bmp085_driver
bmp085
chr_6dm
# generic algorithms:
geometry
filters
# sensor actor support:
i2c serial logger opcd_params scl_voltage threads ltime lmath scl
# ARCADE libraries
glib-2.0
util network_messages
scl_messages
# external libraries:
meschach zmq yaml-cpp rt pthread m protobuf-c)

INSTALL_TARGETS(/bin core)
