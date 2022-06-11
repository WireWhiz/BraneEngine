

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Editor)
message(${ICON_FILE})
if(WIN32)
    add_executable(BraneEditor WIN32 main.cpp ${PROJECT_SOURCE_DIR}/media/brane.rc )
else() #TODO add exe icon on linux
    add_executable(BraneEditor main.cpp)
endif()
target_include_directories(BraneEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_compile_definitions(BraneEditor PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneEditor PRIVATE ecs networking config graphics assets imgui runtime editor_gui)

configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fa-solid-900.ttf DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
FILE(COPY ${PROJECT_SOURCE_DIR}/media/branelogo.png DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(ui)