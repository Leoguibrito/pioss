add_library(piossutil STATIC memalloc.c logger.c timing.c screen.c sha256.c)

target_include_directories(piossutil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
