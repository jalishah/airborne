ADD_LIBRARY(model STATIC model.c adams4.c)
TARGET_LINK_LIBRARIES(model filters geometry)
