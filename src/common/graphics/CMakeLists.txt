#src/client/graphics
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)

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
target_link_libraries(graphics PUBLIC utility BraneScriptRuntime Vulkan::Vulkan glfw::glfw glm::glm)
target_include_directories(graphics PUBLIC ${stb_INCLUDE_DIRS})
target_compile_definitions(graphics PRIVATE ICON_IMAGE_PATH="branelogo.png" CLIENT)