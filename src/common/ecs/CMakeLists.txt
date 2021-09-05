# src/common/ecs

file(GLOB_RECURSE ecs_sources *.cpp)
add_library(ecs STATIC ${ecs_sources})

target_link_libraries(ecs PUBLIC asmjit)
target_include_directories(ecs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
