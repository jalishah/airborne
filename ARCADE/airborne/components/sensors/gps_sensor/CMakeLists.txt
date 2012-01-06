
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../common/messages)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../../common/src/messages)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../../common/src/util)

ADD_EXECUTABLE(gps_sensor main.c gmath.c info.c context.c nmea parse.c parser.c sentence.c time.c tok.c)
TARGET_LINK_LIBRARIES(gps_sensor m)
