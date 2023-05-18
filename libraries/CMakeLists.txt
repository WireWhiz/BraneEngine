
# OpenGL SDK
if(UNIX)
    set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)
# Vulkan SDK
find_package(Vulkan REQUIRED)
# OpenSSL
find_package(OpenSSL REQUIRED)

# stb
add_subdirectory(stb)
# tfd (tinyFileDialog)
add_subdirectory(tfd)
# GLFW
add_subdirectory(glfw)
# sqlite3
add_subdirectory(sqlite3)
# robin_hood
add_subdirectory(robin_hood)

# ImGuizmo
add_library()
