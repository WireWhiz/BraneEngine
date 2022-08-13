#src/tests
set(CMAKE_FOLDER ${CMAKE_FOLDER}/Tests)
add_executable(tests
        "ecs/ecsTest.cpp"
        "ecs/virtualTypeTest.cpp"
        "jit/jitTest.cpp"
        "assets/assetsTest.cpp"
        "utility/threadPool.cpp"
        networking/networking.cpp
        utility/hex.cpp)
include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tests PUBLIC runtime gtest_main ecs_test assets_server networking jsoncpp_static config)
target_compile_definitions(tests PUBLIC TEST_BUILD)

include(GoogleTest)
gtest_discover_tests(tests)