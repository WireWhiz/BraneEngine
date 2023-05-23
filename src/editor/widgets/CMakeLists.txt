set(CMAKE_FOLDER ${CMAKE_FOLDER}/Widgets)
add_library(editorWidgets STATIC
        assetBrowserWidget.cpp
        virtualVariableWidgets.cpp
        assetSelectWidget.cpp
        assetSearchWidget.cpp)
target_link_libraries(editorWidgets PUBLIC ui editorWindows robin_hood shaderc shaderc_util)
target_compile_definitions(editorWidgets PUBLIC CLIENT)