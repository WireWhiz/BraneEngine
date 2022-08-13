set(CMAKE_FOLDER ${CMAKE_FOLDER}/Windows)
add_library(editorWindows STATIC
        loginWindow.cpp
        dataWindow.cpp
        consoleWindow.cpp
        entitiesWindow.cpp
        renderWindow.cpp
        assetBrowserWindow.cpp
        memoryManagerWindow.cpp
        createAssetWindow.cpp)
target_link_libraries(editorWindows PUBLIC ui editorWidgets imgui imguizmo)
target_compile_definitions(editorWindows PUBLIC CLIENT)