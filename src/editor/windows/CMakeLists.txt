add_library(editorWindows STATIC
        loginWindow.cpp
        dataWindow.cpp
        consoleWindow.cpp
        entitiesWindow.cpp
        renderWindow.cpp
        assetBrowserWindow.cpp
        memoryManagerWindow.cpp
        memoryManagerWindow.h
        editorEvents.cpp)
target_link_libraries(editorWindows PUBLIC ui)