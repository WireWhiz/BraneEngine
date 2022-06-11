# Main CMakeListsfor BraneEngine

cmake_minimum_required (VERSION 3.2)

project ("BraneEngine")

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

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

set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/media/brane.res)

set(CLIENT_DEFS CLIENT)
set(RUNTIME_SERVER_DEFS RUNTIME_SERVER)
set(ASSET_SERVER_DEFS ASSET_SERVER)


add_subdirectory(libraries)
add_subdirectory(src)


