#src/assetServer

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(BraneAssetServer main.cpp)
target_include_directories(BraneAssetServer PRIVATE 
	${INCS}
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(BraneAssetServer PRIVATE config networking)
target_compile_definitions(BraneAssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})

file(REMOVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})