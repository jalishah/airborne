
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../common/messages)
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../../common/src/messages)
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../../common/src/util)
SET(CMAKE_C_FLAGS "-g")
ADD_EXECUTABLE(gps_sensor main.c info.c context.c nmea parse.c parser.c sentence.c tok.c)
TARGET_LINK_LIBRARIES(gps_sensor
daemon serial opcd_params scl_messages threads
scl protobuf-c zmq yaml-cpp glib-2.0 m)
