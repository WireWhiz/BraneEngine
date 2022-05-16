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

add_subdirectory(SQLite)