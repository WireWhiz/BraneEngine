﻿# Main CMakeListsfor BraneEngine

cmake_minimum_required (VERSION 3.2)

project ("BraneEngine")

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_TESTS "Include testing build targets" OFF)



if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

# Library Complie Definitions
add_compile_definitions(GLM_FORCE_RADIANS
                GLM_FORCE_DEPTH_ZERO_TO_ONE
                ASIO_STANDALONE
                STATIC_CONCPP)


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    add_compile_definitions(_64BIT)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    add_compile_definitions(_32BIT)

endif()


if(win32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif(win32)

set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/media/brane.res)

set(CLIENT_DEFS CLIENT)
set(RUNTIME_SERVER_DEFS RUNTIME_SERVER)
set(ASSET_SERVER_DEFS ASSET_SERVER)

add_subdirectory(libraries)
add_subdirectory(src)


