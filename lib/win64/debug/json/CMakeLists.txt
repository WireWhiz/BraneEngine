# include/lib/win64/debug
add_library(json INTERFACE)
target_link_libraries(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp.lib)