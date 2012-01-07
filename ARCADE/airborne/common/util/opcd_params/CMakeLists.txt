FIND_PACKAGE(Glib REQUIRED)
INCLUDE_DIRECTORIES(${Glib_INCLUDE_DIRS})
add_library(opcd_params opcd_params.c)
