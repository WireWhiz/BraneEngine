add_library(editor_gui STATIC
    editorUI.cpp
        editorWindow.cpp
        editorEvent.cpp
        windows/loginWindow.cpp
        windows/assetDataWindow.cpp
        windows/consoleWindow.cpp
        windows/entitiesWindow.cpp
        windows/renderWindow.cpp
        windows/assetBrowserWindow.cpp
        windows/memoryManagerWindow.cpp windows/memoryManagerWindow.h)
target_link_libraries(editor_gui PUBLIC imgui runtime)
