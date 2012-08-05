
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
scl_cmd
# system:
control
model
# sensor/actor:
sensor_actor_interfaces
holger_fc
rc_dsl
scl_gps
maxsonar
bmp085
chr_6dm
platform
# generic algorithms:
geometry
filters
# sensor actor support:
i2c serial logger opcd_params voltage threads ltime lmath scl
# ARCADE libraries
glib-2.0
util network_messages
scl_messages
# external libraries:
meschach zmq yaml-cpp rt pthread m protobuf-c)

INSTALL_TARGETS(/bin core)
