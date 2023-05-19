# src/common/utlity

set(SOURCES
		clock.cpp
		threadPool.cpp
		sharedRecursiveMutex.cpp
		serializedData.cpp
		jsonVersioner.cpp enumNameMap.h)

add_library(utility STATIC ${SOURCES} jsonTypeUtilities.h)

target_link_libraries(utility PUBLIC runtime robin_hood jsoncpp_static)

if(BUILD_TESTS)
 add_library(utility_test STATIC ${SOURCES})
target_compile_definitions(utility_test PUBLIC TEST_BUILD)
target_link_libraries(utility_test PUBLIC runtimeexternal_include_dirs jsoncpp_static)
endif()
