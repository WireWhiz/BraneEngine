
add_subdirectory(widgets)
add_subdirectory(windows)
add_subdirectory(gltf)

if(WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL Release)
    add_executable(BraneEditor WIN32
            main.cpp
            ${PROJECT_SOURCE_DIR}/media/brane.rc
            editor.cpp
            serverFilesystem.cpp
            editorEvents.cpp
            )
else() #TODO add exe icon on linux
    add_executable(BraneEditor
            main.cpp
            editor.cpp
            serverFilesystem.cpp
            editorEvents.cpp assetEditorContext.cpp assetEditorContext.h)
endif()
target_include_directories(BraneEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_compile_definitions(BraneEditor PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneEditor PUBLIC ecs networking config graphics assets_client runtime editorWindows systems fileManager gltf)

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/braneEditor)

install(TARGETS BraneEditor
        RUNTIME
        DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)
install(FILES config.json ${PROJECT_SOURCE_DIR}/media/branelogo.png
        DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)
install(DIRECTORY defaultAssets DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)
install(DIRECTORY fonts DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)

add_custom_command(TARGET BraneEditor POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cmake --install . --component Editor --prefix ${CMAKE_BINARY_DIR}
        COMMENT "Installing Editor...")
