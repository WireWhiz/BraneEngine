# src/common/ecs

file(GLOB_RECURSE ecs_sources *.cpp)
add_library(ecs STATIC ${ecs_sources})

target_link_libraries(ecs PUBLIC asmjit assets utility)
target_include_directories(ecs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ecs_test STATIC ${ecs_sources})

target_link_libraries(ecs_test PUBLIC asmjit assets utility_test  -lmcheck -fsanitize=undefined -Wextra)
target_include_directories(ecs_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ecs_test PUBLIC TEST_BUILD MALLOC_CHECK_)
