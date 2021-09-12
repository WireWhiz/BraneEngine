# src/client

add_subdirectory(graphics)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/client)
add_executable(BraneSurfer main.cpp)
target_include_directories(BraneSurfer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(BraneSurfer PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneSurfer PRIVATE ecs graphics networking config)


file(REMOVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})