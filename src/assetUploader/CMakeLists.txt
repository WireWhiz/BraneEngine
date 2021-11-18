# src/assetUploader
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetUploader)
add_executable(BraneAssetUploader main.cpp)
target_include_directories(BraneAssetUploader PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(BraneAssetUploader PRIVATE ${COMMON_DEFS})
target_link_libraries(BraneAssetUploader PRIVATE ecs networking config)