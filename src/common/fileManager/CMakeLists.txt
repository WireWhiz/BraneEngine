# src/common/fileManager
find_package(jsoncpp REQUIRED)
add_library(fileManager STATIC fileManager.cpp fileWatcher.cpp)

get_property(includes GLOBAL PROPERTY INCS)
target_link_libraries(fileManager PUBLIC tfd jsoncpp::jsoncpp)