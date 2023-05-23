
# tfd (tinyFileDialogs) library
add_library(tfd STATIC
        include/tfd/tinyfiledialogs.h
        src/tfd/tinyfiledialogs.c)
target_include_directories(tfd PUBLIC
        include)