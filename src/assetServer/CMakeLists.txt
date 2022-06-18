#src/assetServer

add_subdirectory(bin)
add_subdirectory(database)
add_subdirectory(gltf)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(AssetServer main.cpp assetProcessing/AssetBuilder.cpp assetProcessing/AssetBuilder.h assetNetworking/assetServer.cpp assetNetworking/assetServer.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AssetServer PRIVATE runtime config networking ecs utility fileManager database gltf)
target_compile_definitions(AssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS} ASIO_STANDALONE)


configure_file(config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json COPYONLY)

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database)
