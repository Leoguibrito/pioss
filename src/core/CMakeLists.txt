add_library(piosscore STATIC pioss.c param.c mds.c cli.c dts.c csvexporter.c exporter.c workload.c)

target_link_libraries(piosscore PUBLIC piossutil piossfiledist m)

target_include_directories(piosscore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
