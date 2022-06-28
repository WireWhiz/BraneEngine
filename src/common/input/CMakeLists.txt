add_library(input STATIC
        input.cpp
        input.h
        )
target_link_libraries(input PUBLIC graphics)