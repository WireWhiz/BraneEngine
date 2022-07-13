
add_subdirectory(windows)

if(WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL Release)
    add_executable(BraneEditor WIN32
            main.cpp
            ${PROJECT_SOURCE_DIR}/media/brane.rc
            editor.cpp
            )
else() #TODO add exe icon on linux
    add_executable(BraneEditor
            main.cpp
            editor.cpp)
endif()
target_include_directories(BraneEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_compile_definitions(BraneEditor PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneEditor PRIVATE ecs networking config graphics assets runtime editorWindows systems fileManager)

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/braneEditor)

install(TARGETS BraneEditor
        RUNTIME
        DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)
install(FILES config.json fa-solid-900.ttf ${PROJECT_SOURCE_DIR}/media/branelogo.png
        DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)
install(DIRECTORY shaders DESTINATION ${INSTALL_DIR}
        COMPONENT Editor)

add_custom_command(TARGET BraneEditor POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cmake --install . --component Editor --prefix ${CMAKE_BINARY_DIR}
        COMMENT "Installing Editor...")
