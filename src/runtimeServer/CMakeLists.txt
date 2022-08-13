# src/runtimeServer
set(CMAKE_FOLDER ${CMAKE_FOLDER}/RuntimeServer)
add_executable(RuntimeServer main.cpp
        #"runtimeServer.cpp"
        )
target_compile_definitions(RuntimeServer PRIVATE ${COMMON_DEFS})

file(REMOVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(${CMAKE_INSTALL_PREFIX})

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/runtimeServer)

install(TARGETS RuntimeServer
        RUNTIME
        DESTINATION ${INSTALL_DIR}
        COMPONENT RuntimeServer)
install(FILES config.json
        DESTINATION ${INSTALL_DIR}
        COMPONENT RuntimeServer)