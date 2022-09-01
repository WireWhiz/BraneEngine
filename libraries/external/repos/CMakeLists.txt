set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_ENABLE_SHARED_CRT ON)

add_library(imgui STATIC
        imgui/imconfig.h
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        imgui/imgui_demo.cpp
        imgui/misc/fonts/binary_to_compressed_c.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/backends/imgui_impl_vulkan.h
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_glfw.h
        imgui/misc/cpp/imgui_stdlib.cpp
        )
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/)
target_include_directories(imgui PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(imgui PUBLIC include_dir)

add_library(imguizmo STATIC
        imguizmo/ImGuizmo.cpp)
target_include_directories(imguizmo PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/
        ${CMAKE_CURRENT_SOURCE_DIR}/imguizmo/)
target_link_libraries(imguizmo PUBLIC imgui)

add_library(external_include_dirs INTERFACE)

add_subdirectory(asmjit)
target_compile_definitions(asmjit PUBLIC ASMJIT_EMBED)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if(BUILD_TESTS)
    add_subdirectory(gtest)
endif()
add_subdirectory(SQLite)

add_subdirectory(spirv-headers)
add_subdirectory(spirv-tools)
add_subdirectory(glslang)

add_subdirectory(shaderc)

add_subdirectory(jsoncpp)

target_include_directories(external_include_dirs INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/asio/asio/include
        ${CMAKE_CURRENT_SOURCE_DIR}/asmjit/src
        ${CMAKE_CURRENT_SOURCE_DIR}/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/entt/single_include
        ${CMAKE_CURRENT_SOURCE_DIR}/gtest/googletest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libshaderc/include)
