add_library(scripting STATIC transforms.cpp scripting.cpp scripting.h typeSerializer.cpp typeSerializer.h wasmScriptingTests.cpp)
target_link_libraries(scripting PUBLIC ecs BraneScript)

find_package(wasmer REQUIRED)
find_package(robin_hood REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/wasmTests)
add_executable(wasmScriptingTests wasmScriptingTests.cpp)
target_link_libraries(wasmScriptingTests PUBLIC wasmer::wasmer module_metadata_parser)
target_include_directories(wasmScriptingTests PUBLIC ${wasmer_INCLUDE_DIRS} ${robin_hood_INCLUDE_DIRS})

set(BUILT_WASM ${CMAKE_CURRENT_SOURCE_DIR}/TestWasmScripts/target/wasm32-unknown-unknown/debug/wasm_crate.wasm)
if(EXISTS ${BUILT_WASM})
    file(COPY ${BUILT_WASM} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_subdirectory(module_metadata_parser)