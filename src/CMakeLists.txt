#file: src


file(GLOB_RECURSE LSOURCES *.cpp *.h)

message("SOURCES: " ${SOURCES})

add_executable(BraneEngine ${LSOURCES} ${SOURCES} )
target_include_directories(BraneEngine PRIVATE 
	${INCS}
	${CMAKE_CURRENT_SOURCE_DIR}
	ECS
	Tests
)
message("LIBS: " ${LIBS})
target_link_libraries(BraneEngine PRIVATE ${LIBS})