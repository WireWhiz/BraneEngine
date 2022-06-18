#file: src


include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/common
	../include
)

# run child cmake lists
add_subdirectory(common)

# create executables
add_subdirectory(tests)
add_subdirectory(client)
add_subdirectory(editor)
add_subdirectory(runtimeServer)
add_subdirectory(assetServer)