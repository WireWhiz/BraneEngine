#src/tests

add_executable(tests
        "ecs/ecs_test.cpp"
        "ecs/virtualType_test.cpp"
        "jit/jit_test.cpp"
        "assets/assets_test.cpp"
        "utility/threadPool.cpp"
        networking/networking.cpp
        utility/hex.cpp
        ecs/entt_reflection_test.cpp)
include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tests PUBLIC runtime gtest_main ecs_test assets networking jsoncpp_static config)
target_compile_definitions(tests PUBLIC TEST_BUILD)

include(GoogleTest)
gtest_discover_tests(tests)