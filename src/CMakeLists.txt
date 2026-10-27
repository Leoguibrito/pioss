include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(util)
add_subdirectory(core)
add_subdirectory(file_dists)

add_executable(piosslite main.c)

target_link_libraries(piosslite PUBLIC piossutil piosscore)
