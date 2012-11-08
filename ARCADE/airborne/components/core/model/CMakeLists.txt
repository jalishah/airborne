ADD_LIBRARY(model STATIC madgwick_ahrs.c model.c)
TARGET_LINK_LIBRARIES(model geometry)
