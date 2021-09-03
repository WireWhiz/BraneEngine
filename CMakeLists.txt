# Main CMakeListsfor BraneEngine

cmake_minimum_required (VERSION 3.2)

project ("BraneEngine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Library Complie Definitions
add_compile_definitions(ASMJIT_STATIC
                        GLFW_INCLUDE_VULKAN
                        GLM_FORCE_RADIANS
                        GLM_FORCE_DEPTH_ZERO_TO_ONE)

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

set(INCS "")
set(LIBS "")

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(src)





file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})