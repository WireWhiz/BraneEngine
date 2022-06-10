# include/lib/unix/json
add_library(json INTERFACE)
target_link_libraries(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libjsoncpp.a)