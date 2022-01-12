#src/assetServer

add_subdirectory(bin)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(AssetServer main.cpp "assetNetworking/networkAuthenticator.h" "assetNetworking/networkAuthenticator.cpp")
target_include_directories(AssetServer PRIVATE
	${INCS}
	${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AssetServer PRIVATE config networking ecs utility fileManager http)
target_compile_definitions(AssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})


add_custom_command(TARGET AssetServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/keys ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/keys)

message(${CMAKE_BINARY_DIR}/assetServer)
message(${CMAKE_CURRENT_BINARY_DIR}/config.json)
configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)

file(GLOB_RECURSE WEB_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/pages/*")
message("Found web files: ")
foreach(webFile ${WEB_FILES})
	file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${webFile} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${webFile})
endforeach()
add_custom_command(TARGET AssetServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/pages ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pages)

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database)
