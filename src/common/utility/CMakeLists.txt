# src/common/utlity
find_package(jsoncpp REQUIRED)
find_package(robin_hood REQUIRED)

set(SOURCES
		clock.cpp
		threadPool.cpp
		sharedRecursiveMutex.cpp
		serializedData.cpp
		jsonVersioner.cpp enumNameMap.h)

add_library(utility STATIC ${SOURCES})

target_link_libraries(utility PUBLIC runtime jsoncpp::jsoncpp BraneScriptRuntime)
target_include_directories(utility PUBLIC ${robin_hood_INCLUDE_DIRS})

if(BUILD_TESTS)
 add_library(utility_test STATIC ${SOURCES})
target_compile_definitions(utility_test PUBLIC TEST_BUILD)
target_link_libraries(utility_test PUBLIC runtime jsoncpp::jsoncpp BraneScriptRuntime)
 target_include_directories(utility PUBLIC ${robin_hood_INCLUDE_DIRS})
endif()
