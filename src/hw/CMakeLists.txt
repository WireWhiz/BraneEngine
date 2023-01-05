
add_executable(hw
        test.h
        test.cpp)

target_link_libraries(hw
        PUBLIC project_settings project_headers
        PRIVATE dummy)
