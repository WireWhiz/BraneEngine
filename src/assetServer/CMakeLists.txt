#src/assetServer

add_subdirectory(database)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(BraneAssetServer main.cpp "assetNetworking/networkAuthenticator.h" "assetNetworking/networkAuthenticator.cpp")
target_include_directories(BraneAssetServer PRIVATE 
	${INCS}
	${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(BraneAssetServer PRIVATE config networking ecs utility fileManager http)
target_compile_definitions(BraneAssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})

add_custom_command(TARGET BraneAssetServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/keys ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/keys)

file(GLOB_RECURSE WEB_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/pages/*")
foreach(webFile ${WEB_FILES})
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${webFile} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${webFile} COPYONLY)
endforeach()
add_custom_command(TARGET BraneAssetServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/pages ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pages)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)