# src/common/utlity

add_library(utility STATIC
	clock.cpp
	threadPool.cpp
 "sharedRecursiveMutex.h" "sharedRecursiveMutex.cpp" hex.h staticIndexVector.h serializedData.cpp strCaseCompare.h)

 target_link_libraries(utility PUBLIC include_dir)

 add_library(utility_test STATIC
	clock.cpp
	threadPool.cpp
 "sharedRecursiveMutex.h"
 "sharedRecursiveMutex.cpp"
	 )

target_compile_definitions(utility_test PUBLIC TEST_BUILD)
target_link_options(utility_test PRIVATE -lpthread)