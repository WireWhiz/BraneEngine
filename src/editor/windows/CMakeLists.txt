add_library(editorWindows STATIC
        loginWindow.cpp
        assetDataWindow.cpp
        consoleWindow.cpp
        entitiesWindow.cpp
        renderWindow.cpp
        assetBrowserWindow.cpp
        memoryManagerWindow.cpp
        memoryManagerWindow.h)
target_link_libraries(editorWindows PUBLIC ui)