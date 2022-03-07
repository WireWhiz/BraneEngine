#src/assetServer

add_subdirectory(bin)
add_subdirectory(database)
add_subdirectory(httpServer)
add_subdirectory(gltf)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(AssetServer main.cpp assetProcessing/AssetBuilder.cpp assetProcessing/AssetBuilder.h assetNetworking/assetServer.cpp assetNetworking/assetServer.h)
target_include_directories(AssetServer PRIVATE
	${INCS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AssetServer PRIVATE config networking ecs utility fileManager assetHttpServer gltf)
target_compile_definitions(AssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})


add_custom_command(TARGET AssetServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/keys ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/keys)

configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)

#file(GLOB_RECURSE WEB_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/pages/*")
#message("Found web files: ")
#foreach(webFile ${WEB_FILES})
#	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${webFile} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${webFile})
#endforeach()
#[[add_custom_command(TARGET AssetServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/pages ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pages)]]

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database)
