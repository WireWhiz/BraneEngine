#src/tests

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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

target_link_libraries(tests PUBLIC runtime gtest_main ecs_test assets networking json config)
target_compile_definitions(tests PUBLIC TEST_BUILD)

include(GoogleTest)
gtest_discover_tests(tests)