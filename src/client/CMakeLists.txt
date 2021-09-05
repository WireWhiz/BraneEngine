# src/client

add_subdirectory(graphics)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/client)
add_executable(BraneSurfer main.cpp)
target_include_directories(BraneSurfer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
message("LIBS: " ${LIBS})
target_compile_definitions(BraneSurfer PRIVATE ${COMMON_DEFS} ${CLIENT_DEFS})
target_link_libraries(BraneSurfer PRIVATE 
	ecs 
	graphics
	${LIBS}
)