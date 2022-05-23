add_library(editor_gui STATIC
    editorUI.cpp
        editorWindow.cpp
        EditorEvent.cpp
        windows/LoginWindow.cpp
        windows/AssetDataWindow.cpp
        windows/ConsoleWindow.cpp
        windows/EntitiesWindow.cpp
        windows/RenderWindow.cpp
        windows/AssetBrowserWindow.cpp
        )
target_link_libraries(editor_gui PUBLIC imgui runtime)
