#file: src

set(CMAKE_FOLDER BraneEngine)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/common
	../include
)

add_library(project_properties
		INTERFACE)
target_compile_features(project_properties
		INTERFACE cxx_std_20)

add_library(client_properties INTERFACE)
target_link_libraries(client_properties
		INTERFACE project_properties)
target_compile_definitions(client_properties INTERFACE
		CLIENTSIDE)

add_library(server_properties INTERFACE)
target_link_libraries(server_properties
		INTERFACE project_properties)
target_compile_definitions(server_properties INTERFACE
		SERVERSIDE)

# run child cmake lists
add_subdirectory(common)

# create executables
add_subdirectory(client)
add_subdirectory(editor)
add_subdirectory(runtimeServer)
add_subdirectory(assetServer)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
