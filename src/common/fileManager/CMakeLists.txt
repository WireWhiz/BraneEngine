# src/common/fileManager
add_library(fileManager STATIC fileManager.cpp fileWatcher.cpp)

get_property(includes GLOBAL PROPERTY INCS)
target_link_libraries(fileManager PUBLIC include_dir tfd jsoncpp_static)