# CMakeList.txt : CMake project for BraneEngine, include source and define
# project specific logic here.
#

message("0")
cmake_minimum_required (VERSION 3.8)

message("1")
project ("BraneEngine")

message("2")

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


message("3")

file(GLOB_RECURSE SOURCES src/*.h src/*.cpp include/*.h include/*.cpp)
add_compile_definitions(ASMJIT_STATIC)

message("1")
include_directories(
    src/ECS
    src/Tests
    lib
    include
)


if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("debug mode")
    add_compile_definitions(DEBUG)
    if(win32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS_CUI /ENTRY:mainCRTStartup")
    endif(win32)
else ()
    message("shipping mode")
    if(win32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif(win32)
endif ()


# Add source to this project's executable.
add_executable (BraneEngine ${SOURCES})



if (WIN32)
    message(${CMAKE_CURRENT_SOURCE_DIR}/lib/GLFW/glfw3.lib)
    target_link_libraries(BraneEngine ${CMAKE_CURRENT_SOURCE_DIR}/lib/GLFW/glfw3.lib)
endif (WIN32)
if (UNIX)
    target_link_libraries(BraneEngine -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)

    # add_link_options(-lrt)
    find_library(LIBRT rt) 
    if(LIBRT)
        target_link_libraries(BraneEngine ${LIBRT})
    endif()

endif (UNIX)


target_include_directories(BraneEngine PRIVATE src/ECS)
target_include_directories(BraneEngine PRIVATE src/Tests)
target_include_directories(BraneEngine PRIVATE lib)
target_include_directories(BraneEngine PRIVATE include)



#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})