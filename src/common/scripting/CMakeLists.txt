add_library(scripting STATIC transforms.cpp scripting.cpp scripting.h)
target_link_libraries(scripting PUBLIC ecs BraneScript)