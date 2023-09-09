message(STATUS "Building rust library for ${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    execute_process(COMMAND cargo build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY)
    set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/${CMAKE_STATIC_LIBRARY_PREFIX}scripting_runtime${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    execute_process(COMMAND cargo build --release
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY)
    set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/${CMAKE_STATIC_LIBRARY_PREFIX}scripting_runtime${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

add_library(scripting_runtime_rust INTERFACE)
target_link_libraries(scripting_runtime_rust INTERFACE ${LIB_PATH})
target_include_directories(scripting_runtime_rust INTERFACE c-include)