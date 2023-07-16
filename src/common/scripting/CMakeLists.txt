add_library(scripting STATIC transforms.cpp scripting.cpp scripting.h typeSerializer.cpp typeSerializer.h wasmScriptingTests.cpp)
target_link_libraries(scripting PUBLIC ecs BraneScript)

find_package(wasmer REQUIRED)

add_executable(wasmScriptingTests wasmScriptingTests.cpp)
target_link_libraries(wasmScriptingTests PUBLIC wasmer::wasmer)
target_include_directories(wasmScriptingTests PUBLIC ${wasmer_INCLUDE_DIRS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/TestWasmScripts/wasm/wasm_crate.wasm DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
