add_library(ui STATIC
        gui.cpp
        guiWindow.cpp
        guiEvent.cpp
        guiRenderer.cpp
        guiPopup.cpp
        guiPopup.h)
target_link_libraries(ui PUBLIC imgui imguizmo runtime)
