# src/common/utlity

set(SOURCES
		clock.cpp
		threadPool.cpp
		sharedRecursiveMutex.cpp
		serializedData.cpp
		jsonVersioner.cpp)

add_library(utility STATIC ${SOURCES})

target_link_libraries(utility PUBLIC runtime include_dir jsoncpp_static)

if(BUILD_TESTS)
 add_library(utility_test STATIC ${SOURCES})
target_compile_definitions(utility_test PUBLIC TEST_BUILD)
target_link_options(utility_test PUBLIC runtime include_dir jsoncpp_static)
endif()
