#include "window.h"

namespace graphics
{
    Window::Window()
    {
        init();
    }

    Window::~Window()
    {
        cleanup();
    }

    void Window::createSurface(VkInstance instance, const VkAllocationCallbacks* allocator)
    {
        if (glfwCreateWindowSurface(instance, _window, nullptr, &_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Window::destroySurface(VkInstance instance, const VkAllocationCallbacks* allocator)
    {
        vkDestroySurfaceKHR(instance, _surface, nullptr);
    }

    VkSurfaceKHR Window::surface()
    {
        return _surface;
    }

    void Window::init()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(800, 600, "Brane Surfer", nullptr, nullptr);
        assert(_window != NULL);


    }

    void Window::update()
    {
        glfwPollEvents();
        glfwGetFramebufferSize(_window, &_size.x, &_size.y);
    }

    bool Window::closed()
    {
        return glfwWindowShouldClose(_window);
    }

    glm::ivec2 Window::size()
    {
        return _size;
    }

    void Window::cleanup()
    {

        glfwDestroyWindow(_window);

        glfwTerminate();
    }
}