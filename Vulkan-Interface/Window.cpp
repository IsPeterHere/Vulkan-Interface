#include <stdexcept>
#include "MYR.h"

Window::Window(uint32_t width, uint32_t height) : WIDTH(width), HEIGHT(height) {}
Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//Do not create an OpenGL context 
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//Do not allow resizing of window

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    if (window == NULL)
    {
        throw std::runtime_error("failed to GLFW window!");
    }
}
