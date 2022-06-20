#src/common/config

add_library(config STATIC config.cpp)
target_link_libraries(config PUBLIC jsoncpp_static include_dir)