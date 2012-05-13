SET(COMMON_FLAGS "-Wall -Wextra -pipe -O3 -mcpu=cortex-a8 -ftree-vectorize -mfpu=neon -mfloat-abi=softfp -ffast-math -fsingle-precision-constant -funroll-loops")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
