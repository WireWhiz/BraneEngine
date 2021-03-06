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
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/braneSurfer)

install(TARGETS BraneSurfer
		RUNTIME
		DESTINATION ${INSTALL_DIR}
		COMPONENT Client)
install(FILES config.json
		DESTINATION ${INSTALL_DIR}
		COMPONENT Client)
install(DIRECTORY shaders DESTINATION ${INSTALL_DIR}
		COMPONENT Client)

add_custom_command(TARGET BraneSurfer POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND cmake --install . --component Client --prefix ${CMAKE_BINARY_DIR}
		COMMENT "Installing Client...")