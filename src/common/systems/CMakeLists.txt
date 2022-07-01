add_library(systems STATIC transforms.cpp)
target_link_libraries(systems PUBLIC ecs)