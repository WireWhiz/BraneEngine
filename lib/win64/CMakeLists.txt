# lib/win64

add_subdirectory(shared)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_subdirectory(debug)
else ()
    add_subdirectory(release)
endif()