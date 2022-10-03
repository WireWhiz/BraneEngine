add_library(chunks STATIC
        chunkLoader.cpp)
target_link_libraries(chunks PUBLIC include_dir ecs runtime utility)