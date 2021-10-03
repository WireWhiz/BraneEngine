# src/runtimeServer
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/runtimeServer)
add_executable(BraneRuntimeServer main.cpp "runtimeServer.cpp")

message("LIBS: " ${LIBS})
target_link_libraries(BraneRuntimeServer PRIVATE ${LIBS})
target_compile_definitions(BraneRuntimeServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})


file(REMOVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})