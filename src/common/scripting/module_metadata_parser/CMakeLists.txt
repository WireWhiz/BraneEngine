message(STATUS "Building rust library for ${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    execute_process(COMMAND cargo build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY)
    set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/${CMAKE_SHARED_LIBRARY_PREFIX}module_metadata_parser${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    execute_process(COMMAND cargo build --release
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY)
    set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target/debug/${CMAKE_SHARED_LIBRARY_PREFIX}module_metadata_parser${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

file(COPY ${LIB_PATH} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_library(module_metadata_parser INTERFACE)
target_link_libraries(module_metadata_parser INTERFACE ${LIB_PATH})
target_include_directories(module_metadata_parser INTERFACE include)