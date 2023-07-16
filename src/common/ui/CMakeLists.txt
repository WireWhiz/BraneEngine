find_package(imgui REQUIRED)
find_package(imguizmo REQUIRED)

add_library(ui STATIC
        gui.cpp
        guiWindow.cpp
        guiEvent.cpp
        guiRenderer.cpp
        guiPopup.cpp
        guiPopup.h)
target_link_libraries(ui PUBLIC imgui::imgui imgui_extras imguizmo::imguizmo runtime graphics)
target_include_directories(ui PUBLIC ${imgui_extras_INCLUDE_DIRS})
message("Current include dirs: ${INCLUDE_DIRS}")