add_library(editor_gui STATIC
    editorUI.cpp
        editorWindow.cpp
        windows/LoginWindow.cpp
        )
target_link_libraries(editor_gui PUBLIC imgui runtime)