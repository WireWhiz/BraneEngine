# VulkanSDK
find_package(Vulkan REQUIRED)
# glslang
add_subdirectory(glslang)
# SPIRV Headers
set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
add_subdirectory(SPIRV-Headers)
# SPIRV Tools
set(SPIRV_SKIP_EXECUTABLES ON)
set(SPIRV_SKIP_TESTS ON)
add_subdirectory(SPIRV-Tools)
# SPIRV Cross
option(SPIRV_CROSS_ENABLE_TESTS "Enable SPIRV-Cross tests." OFF)
add_subdirectory(SPIRV-Cross)
# shaderc
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_ENABLE_SHARED_CRT ON)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
add_subdirectory(shaderc)
target_include_directories(glslc PUBLIC
        shaderc/glslc/src)

# stb
add_subdirectory(stb)
# tfd (tinyFileDialog)
add_subdirectory(tfd)
# sqlite3
add_subdirectory(sqlite3)
# jsoncpp
add_subdirectory(jsoncpp)
# robin_hood
add_subdirectory(robin_hood)

# ImGUI
add_library(imgui STATIC
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/misc/cpp/imgui_stdlib.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.cpp)
target_include_directories(imgui PUBLIC
        imgui)
target_link_libraries(imgui PUBLIC
        glfw Vulkan::Vulkan)
# ImGuizmo
add_library(imguizmo STATIC
        imguizmo/GraphEditor.cpp
        imguizmo/ImCurveEdit.cpp
        imguizmo/ImGradient.cpp
        imguizmo/ImGuizmo.cpp
        imguizmo/ImSequencer.cpp)
target_include_directories(imguizmo PUBLIC
        imguizmo)
target_link_libraries(imguizmo PUBLIC
        imgui glfw)
