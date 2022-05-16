# src/client

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/BraneSurfer)
add_executable(BraneSurfer main.cpp "client.cpp")
target_include_directories(BraneSurfer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(BraneSurfer PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneSurfer PRIVATE runtime ecs networking config graphics assets)

file(COPY shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)
