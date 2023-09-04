find_package(robin_hood REQUIRED)

add_library(scripting STATIC
        #transforms.cpp
        scripting.cpp
        #typeSerializer.cpp
)
target_link_libraries(scripting PUBLIC
        #ecs
        scripting_runtime_rust
)
target_include_directories(scripting PUBLIC ${robin_hood_INCLUDE_DIRS})


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/wasmTests)
add_executable(wasmScriptingTests wasmScriptingTests.cpp)
target_link_libraries(wasmScriptingTests PUBLIC scripting)

add_subdirectory(scripting_runtime)