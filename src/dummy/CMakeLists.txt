# Dummy Library

add_library(dummy
        dummy.h
        dummy.cpp)
target_link_libraries(dummy
        PUBLIC project_settings)