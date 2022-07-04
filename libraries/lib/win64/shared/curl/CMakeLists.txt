add_library(curl INTERFACE)
target_link_libraries(curl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libcurl.lib)