ADD_SUBDIRECTORY(basic)
ADD_SUBDIRECTORY(position)
ADD_SUBDIRECTORY(util)

ADD_LIBRARY(control STATIC control.c)
TARGET_LINK_LIBRARIES(control basic_control position_control control_util)
