# src/common/ecs

add_library(ecs STATIC
        core/virtualType.cpp
        core/archetype.cpp
        core/chunk.cpp
        core/component.cpp
        core/componentSet.cpp
        core/entity.cpp
        core/entitySet.cpp
        core/systemManager.cpp
        core/archetypeManager.cpp
        core/componentManager.cpp)
target_link_libraries(ecs PUBLIC runtime assets utility)
target_include_directories(ecs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ecs_test STATIC
        core/archetype.cpp
        core/component.cpp
        core/componentSet.cpp
        core/chunk.cpp
        core/entity.cpp
        core/virtualType.cpp
        core/entitySet.cpp
        core/systemManager.cpp
        core/archetypeManager.cpp
        core/componentManager.cpp)
target_link_libraries(ecs_test PUBLIC runtime assets utility_test)
target_include_directories(ecs_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ecs_test PUBLIC TEST_BUILD MALLOC_CHECK_)
