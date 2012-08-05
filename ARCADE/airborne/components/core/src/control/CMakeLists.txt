ADD_SUBDIRECTORY(lib)

ADD_LIBRARY(control STATIC z_ctrl.c ctrl.c navi.c yaw_ctrl.c)
TARGET_LINK_LIBRARIES(control control_lib)
