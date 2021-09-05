#file: src


include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/common
	../include
)

# run child cmake lists
add_subdirectory(common)

# create executables
add_subdirectory(client)


set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/runtimeServer)
add_executable(BraneRuntimeServer runtimeServer/main.cpp)

message("LIBS: " ${LIBS})
target_link_libraries(BraneRuntimeServer PRIVATE ${LIBS})
target_compile_definitions(BraneRuntimeServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assetServer)
add_executable(BraneAssetServer assetServer/main.cpp)
target_include_directories(BraneAssetServer PRIVATE 
	${INCS}
	${CMAKE_CURRENT_SOURCE_DIR}
	ECS
	Tests
)
target_link_libraries(BraneAssetServer PRIVATE ${LIBS})
target_compile_definitions(BraneAssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS})