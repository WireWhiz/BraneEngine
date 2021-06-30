# CMakeList.txt : CMake project for BraneEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("BraneEngine")


set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES src/*.h src/*.cpp src/*.hh src/*.cc)
add_compile_definitions(ASMJIT_STATIC)

include_directories(
    src/ECS
    src/Tests
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("debug mode")
    add_compile_definitions(DEBUG)
else ()
    message("shipping mode")
endif ()
    

if (UNIX)
    add_link_options(-lrt)
endif (UNIX)


# Add source to this project's executable.
add_executable (BraneEngine ${SOURCES})

target_include_directories(BraneEngine PRIVATE ${YOUR_DIRECTORY}/src/ECS)
target_include_directories(BraneEngine PRIVATE ${YOUR_DIRECTORY}/src/Tests)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})