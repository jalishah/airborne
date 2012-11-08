ADD_LIBRARY(model STATIC mag_decl.c madgwick_ahrs.c model.c)
TARGET_LINK_LIBRARIES(model geometry)
