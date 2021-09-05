# Main CMakeListsfor BraneEngine

cmake_minimum_required (VERSION 3.2)

project ("BraneEngine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library Complie Definitions
set(COMMON_DEFS GLM_FORCE_RADIANS
                GLM_FORCE_DEPTH_ZERO_TO_ONE)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("debug mode")
    list(APPEND COMMON_DEFS DEBUG)
    if(win32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS_CUI /ENTRY:mainCRTStartup")
    endif(win32)
else ()
    message("shipping mode")
    if(win32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif(win32)
endif ()


set(CLIENT_DEFS CLIENT)
set(RUNTIME_SERVER_DEFS RUNTIME_SERVER)
set(ASSET_SERVER_DEFS ASSET_SERVER)

set(LIBS "")


# Run subdir cmakelists
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(src)


# Copy needed files to build folders
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/client/shaders)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/client)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/client)