# src/common/fileManager
add_library(fileManager STATIC fileManager.cpp)

target_link_libraries(fileManager PUBLIC assets)