
# RobinHood library
add_library(robin_hood INTERFACE
        include/robin_hood/robin_hood.h)
target_include_directories(robin_hood INTERFACE
        include)
set_target_properties(robin_hood
        PROPERTIES LINKER_LANGUAGE CXX)