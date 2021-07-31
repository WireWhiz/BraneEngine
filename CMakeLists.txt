# CMakeList.txt : CMake project for BraneEngine, include source and define
# project specific logic here.
#

cmake_minimum_required (VERSION 3.8)

project ("BraneEngine")


set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)



file(GLOB_RECURSE SOURCES src/*.cpp include/*.cpp)
add_compile_definitions(ASMJIT_STATIC)

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
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        file(GLOB_RECURSE LIB_SOURCES lib/win64/shared/*.lib lib/win64/debug/*.lib)
    else ()
        file(GLOB_RECURSE LIB_SOURCES lib/win64/shared/*.lib lib/win64/release/*.lib)
    endif()
    message(${LIB_SOURCES})
    target_link_libraries(BraneEngine ${LIB_SOURCES})
endif (WIN32)
if (UNIX)
    target_link_libraries(BraneEngine -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lglslang -lSPIRV -lHLSL )

    file(GLOB_RECURSE LIB_SOURCES lib/unix/*.a)
    message(${LIB_SOURCES})
    target_link_libraries(BraneEngine ${LIB_SOURCES})
    
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



#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
        TARGET BraneEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/shaders
                ${CMAKE_CURRENT_BINARY_DIR}/shaders)