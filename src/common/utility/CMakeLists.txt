# src/common/utlity

add_library(utility STATIC
	clock.cpp
	threadPool.cpp
 "shared_recursive_mutex.h" "shared_recursive_mutex.cpp")

 add_library(utility_test STATIC
	clock.cpp
	threadPool.cpp
 "shared_recursive_mutex.h" 
 "shared_recursive_mutex.cpp")

target_compile_definitions(utility_test PUBLIC MALLOC_CHECK_=1)
target_link_options(utility_test PRIVATE -lmcheck)