#include <stdexcept>
#include "MYR.h"

using namespace MYR;

void framebufferResizeCallback(GLFWwindow*, int, int);


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

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    if (window == NULL) 
        throw std::runtime_error("failed to GLFW window!");
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  
}

void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height) 
{
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->windowResized = true;
}