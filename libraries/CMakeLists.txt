set(CMAKE_FOLDER ${CMAKE_FOLDER}/Libraries)
add_subdirectory(tinyFileDialog)
add_subdirectory(imgui)

option(BS_BUILD_EXECUTABLE "Build standalone compiler/interpreter" OFF)
option(BS_BUILD_TESTS "Build tests" OFF)
option(BS_CALL_CONAN "Call conan to install dependencies" OFF)
add_subdirectory(BraneScript)