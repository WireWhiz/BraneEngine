add_library(ui STATIC
        gui.cpp
        guiWindow.cpp
        guiEvent.cpp
        ../../editor/windows/loginWindow.cpp
        ../../editor/windows/dataWindow.cpp
        ../../editor/windows/consoleWindow.cpp
        ../../editor/windows/entitiesWindow.cpp
        ../../editor/windows/renderWindow.cpp
        ../../editor/windows/assetBrowserWindow.cpp
        ../../editor/windows/memoryManagerWindow.cpp
        ../../editor/windows/memoryManagerWindow.h guiRenderer.cpp guiRenderer.h)
target_link_libraries(ui PUBLIC imgui runtime)
