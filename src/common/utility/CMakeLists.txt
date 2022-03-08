# src/common/utlity

add_library(utility STATIC
	clock.cpp
	threadPool.cpp
 "shared_recursive_mutex.h" "shared_recursive_mutex.cpp" hex.h staticIndexVector.h)

 target_link_libraries(utility PRIVATE -lpthread)

 add_library(utility_test STATIC
	clock.cpp
	threadPool.cpp
 "shared_recursive_mutex.h" 
 "shared_recursive_mutex.cpp"
	 )

target_compile_definitions(utility_test PUBLIC TEST_BUILD)
target_link_options(utility_test PRIVATE -lpthread)