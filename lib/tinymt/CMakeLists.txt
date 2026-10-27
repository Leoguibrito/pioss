add_library(tinymt tinymt32.c)

target_include_directories(tinymt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
