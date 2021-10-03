#src/tests

# preface all test libraries with t so ecs would be tecs
# test naming follows the pattern "namespace.class.test_name"

add_executable(test main.cpp)
message("tests enabled!")
enable_testing()

set(TestDir ${CMAKE_CURRENT_BINARY_DIR}/test.exe)
add_subdirectory(ecs)