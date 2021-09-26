#src/assetServer

add_subdirectory(database)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(BraneAssetServer main.cpp "assetNetworking/assetServer.cpp")
target_include_directories(BraneAssetServer PRIVATE 
	${INCS}
	${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(BraneAssetServer PRIVATE config networking database)
target_compile_definitions(BraneAssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})

add_custom_command(TARGET BraneAssetServer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/bin/win64/mysql ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/keys
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(REMOVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})