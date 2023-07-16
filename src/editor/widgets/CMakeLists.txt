set(CMAKE_FOLDER ${CMAKE_FOLDER}/Widgets)
add_library(editorWidgets STATIC
        assetBrowserWidget.cpp
        virtualVariableWidgets.cpp
        assetSelectWidget.cpp
        assetSearchWidget.cpp)
target_link_libraries(editorWidgets PUBLIC ui editorWindows BraneScript imgui_extras)
target_compile_definitions(editorWidgets  PUBLIC CLIENT)
target_include_directories(editorWidgets PUBLIC ${imgui_extras_INCLUDE_DIR})