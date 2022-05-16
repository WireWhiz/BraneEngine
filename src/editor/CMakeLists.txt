

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Editor)
add_executable(BraneEditor main.cpp)
target_include_directories(BraneEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_compile_definitions(BraneEditor PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneEditor PRIVATE ecs networking config graphics assets imgui runtime editor_gui)

configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)

add_subdirectory(ui)