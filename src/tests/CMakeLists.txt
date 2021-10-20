#src/tests

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(gtests 
 "ecs/ecs_test.cpp"
 "ecs/systems_test.cpp"
 "ecs/virtualType_test.cpp"
 "jit/jit_test.cpp"
   "assets/assets_test.cpp")
include_directories(gtests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gtests gtest_main ecs_test jit assets)
target_compile_definitions(gtests PUBLIC TEST_BUILD)

include(GoogleTest)
gtest_discover_tests(gtests)