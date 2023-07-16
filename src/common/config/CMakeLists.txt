#src/common/config
find_package(jsoncpp REQUIRED)

add_library(config STATIC config.cpp)
target_link_libraries(config PUBLIC jsoncpp::jsoncpp)