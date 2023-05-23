set(CMAKE_FOLDER ${CMAKE_FOLDER}/Windows)
find_package(OpenSSL REQUIRED)

add_library(editorWindows STATIC
        dataWindow.cpp
        consoleWindow.cpp
        entitiesWindow.cpp
        renderWindow.cpp
        assetBrowserWindow.cpp
        memoryManagerWindow.cpp
        #createAssetWindow.cpp
        syncWindow.cpp
        selectProjectWindow.cpp editorWindow.cpp editorWindow.h)
target_link_libraries(editorWindows PUBLIC
        imgui ui editorWidgets imguizmo tfd OpenSSL::SSL)
target_compile_definitions(editorWindows PUBLIC CLIENT)