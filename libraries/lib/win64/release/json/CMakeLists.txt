# include/lib/win64/release
add_library(json INTERFACE)
target_link_libraries(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp.lib)