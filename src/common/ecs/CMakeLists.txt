# src/common/ecs

add_library(ecs STATIC
        virtualType.cpp
        archetype.cpp
        chunk.cpp
        component.cpp
        componentSet.cpp
        entity.cpp
        entitySet.cpp
        systemManager.cpp
        archetypeManager.cpp
        componentManager.cpp entityID.cpp entityID.h)
target_link_libraries(ecs PUBLIC runtime utility)
target_include_directories(ecs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTS)
add_library(ecs_test STATIC
        archetype.cpp
        component.cpp
        componentSet.cpp
        chunk.cpp
        entity.cpp
        virtualType.cpp
        entitySet.cpp
        systemManager.cpp
        archetypeManager.cpp
        componentManager.cpp entityID.cpp entityID.h)
target_link_libraries(ecs_test PUBLIC runtime assets utility_test)
target_include_directories(ecs_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ecs_test PUBLIC TEST_BUILD MALLOC_CHECK_)
endif()