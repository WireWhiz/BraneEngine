add_library(editorWidgets STATIC
        assetBrowserWidget.cpp
        virtualVariableWidgets.cpp
        assetSelectWidget.cpp assetSearchWidget.cpp assetSearchWidget.h)
target_link_libraries(editorWidgets PUBLIC ui editorWindows gltf)
target_compile_definitions(editorWidgets  PUBLIC CLIENT)