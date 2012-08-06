ADD_LIBRARY(position_control STATIC att_ctrl.c z_ctrl.c navi.c yaw_ctrl.c)
TARGET_LINK_LIBRARIES(position_control control_util)
