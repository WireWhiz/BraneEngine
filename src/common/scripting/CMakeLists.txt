add_library(scripting STATIC transforms.cpp scripting.cpp scripting.h typeSerializer.cpp typeSerializer.h)
target_link_libraries(scripting PUBLIC ecs BraneScript)