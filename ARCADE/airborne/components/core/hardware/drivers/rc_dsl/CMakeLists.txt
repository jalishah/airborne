ADD_LIBRARY(rc_dsl_driver STATIC simple_kalman.c rc_dsl_driver.c)
TARGET_LINK_LIBRARIES(rc_dsl_driver rc_dsl)
