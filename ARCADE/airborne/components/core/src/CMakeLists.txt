
cmake_minimum_required(VERSION 2.8)

ADD_SUBDIRECTORY(sensor_actor)
ADD_SUBDIRECTORY(controllers)
ADD_SUBDIRECTORY(platform)
ADD_SUBDIRECTORY(algorithms)
ADD_SUBDIRECTORY(scl_cmd)
ADD_SUBDIRECTORY(model)
ADD_SUBDIRECTORY(util)

ADD_EXECUTABLE(core main.c)

TARGET_LINK_LIBRARIES(core
daemon
scl_cmd
sensor_actor_interfaces
# system:
controllers
model
platform
# sensor/actor:
holger_fc
rc_dsl
scl_gps
maxsonar
bmp085
chr_6dm
# generic algorithms:
algorithms
# sensor actor support:
i2c gpio twl4030_madc serial logger opcd_params voltage threads ltime lmath scl
# ARCADE libraries
glib-2.0
util network_messages
scl_messages
# external libraries:
meschach zmq yaml-cpp rt pthread m protobuf-c)

INSTALL_TARGETS(/bin core)
