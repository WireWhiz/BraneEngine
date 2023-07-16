find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)

add_library(imgui_extras STATIC
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_vulkan.cpp
    imgui/misc/cpp/imgui_stdlib.cpp)
target_link_libraries(imgui_extras PUBLIC imgui::imgui glfw::glfw Vulkan::Vulkan)
target_include_directories(imgui_extras PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set(imgui_extras_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)