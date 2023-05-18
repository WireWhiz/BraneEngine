
# RobinHood library
add_library(robin_hood STATIC
        include/robin_hood/robin_hood.h)
target_include_directories(robin_hood PUBLIC
        include)
set_target_properties(robin_hood
        PROPERTIES LINKER_LANGUAGE CXX)