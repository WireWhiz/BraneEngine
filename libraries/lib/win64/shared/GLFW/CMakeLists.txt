# lib/win64/shared/GLFW
add_library(glfw INTERFACE)
target_link_libraries(glfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glfw3.lib)