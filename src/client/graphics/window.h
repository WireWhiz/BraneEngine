#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stdexcept>


namespace graphics
{
	class Window
	{
	private:
		GLFWwindow* _window;
		glm::ivec2 _size;
		VkSurfaceKHR _surface;
		void init();
		void cleanup();
	public:
		Window();
		~Window();
		void createSurface(VkInstance instance, const VkAllocationCallbacks* allocator);
		void destroySurface(VkInstance instance, const VkAllocationCallbacks* allocator);
		VkSurfaceKHR surface();
		void update();
		bool closed();
		glm::ivec2 size();
		void setSize(glm::ivec2& newSize);
	};

}