
cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../common/messages)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../../common/src/messages)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../../common/src/util)

ADD_SUBDIRECTORY(sensor_actor)
ADD_SUBDIRECTORY(controllers)
ADD_SUBDIRECTORY(platform)
ADD_SUBDIRECTORY(algorithms)
ADD_SUBDIRECTORY(interfaces)
ADD_SUBDIRECTORY(model)
ADD_SUBDIRECTORY(util)
#ADD_SUBDIRECTORY(tests)

FIND_PACKAGE(OpenCV REQUIRED)

ADD_EXECUTABLE(core main.c)

TARGET_LINK_LIBRARIES(core
daemon
scl_interfaces
sensor_actor_interfaces
# system:
controllers
model
char_queue
platform
# sensor/actor:
flightgear
mk_fc
rc_dsl
scl_gps
maxsonar
bmp085
leds
chr_6dm
# generic algorithms:
algorithms
# sensor actor support:
i2c gpio twl4030_madc serial logger opcd_params threads ltime lmath scl config
# ARCADE libraries
util network_messages
scl_messages
# external libraries:
cxcore cv meschach zmq yaml-cpp rt pthread m glib-2.0 protobuf-c)

INSTALL_TARGETS(/bin core)
