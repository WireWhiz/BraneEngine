
# stb library
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
        include)
set_target_properties(stb
        PROPERTIES LINKER_LANGUAGE C)