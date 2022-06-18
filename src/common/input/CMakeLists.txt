add_library(input STATIC
        Input.cpp
        Input.h
        )
target_link_libraries(input PUBLIC graphics)