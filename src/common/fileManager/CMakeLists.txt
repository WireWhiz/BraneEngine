# src/common/fileManager
add_library(fileManager STATIC fileManager.h fileManager.cpp)

target_link_libraries(fileManager PUBLIC assets)
get_property(includes GLOBAL PROPERTY INCS)
target_link_libraries(fileManager PUBLIC include_dir tfd)