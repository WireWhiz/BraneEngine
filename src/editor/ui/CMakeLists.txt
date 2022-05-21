add_library(editor_gui STATIC
    editorUI.cpp
        editorWindow.cpp
        windows/LoginWindow.cpp
        windows/AssetDataWindow.cpp
        windows/ConsoleWindow.cpp
        windows/EntitiesWindow.cpp
        windows/RenderWindow.cpp
        )
target_link_libraries(editor_gui PUBLIC imgui runtime)