ADD_LIBRARY(estimators STATIC ahrs.c pos.c)
TARGET_LINK_LIBRARIES(estimators geometry)
