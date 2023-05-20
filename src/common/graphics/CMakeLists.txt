#src/client/graphics

# OpenGL SDK
if(UNIX)
	set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)
# Vulkan SDK
find_package(Vulkan REQUIRED)

add_library(graphics STATIC
		graphics.cpp
		graphicsDevice.cpp
		"material.cpp"
		"mesh.cpp"
		renderer.cpp
		shader.cpp
		swapChain.cpp
		texture.cpp
		window.cpp
		renderTarget.cpp
		camera.cpp
		sceneRenderer.cpp
		graphicsBuffer.cpp)
target_compile_definitions(graphics PUBLIC GLFW_INCLUDE_VULKAN ${COMMON_DEFS})
target_link_libraries(graphics PUBLIC utility stb glfw utility Vulkan::Vulkan)
target_compile_definitions(graphics PRIVATE ICON_IMAGE_PATH="branelogo.png" CLIENT)
if(WIN32)
target_link_libraries(graphics PUBLIC vulkan glfw)
else (WIN32)
target_link_libraries(graphics PUBLIC -lvulkan -lglfw -lX11 -lXxf86vm -lXrandr -lXi)
endif (WIN32)