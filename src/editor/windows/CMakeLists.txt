add_library(editorWindows STATIC
        loginWindow.cpp
        dataWindow.cpp
        consoleWindow.cpp
        entitiesWindow.cpp
        renderWindow.cpp
        assetBrowserWindow.cpp
        memoryManagerWindow.cpp
        memoryManagerWindow.h
        editorEvents.cpp createAssetWindow.cpp createAssetWindow.h)
target_link_libraries(editorWindows PUBLIC ui editorWidgets)
target_compile_definitions(editorWindows PUBLIC CLIENT)