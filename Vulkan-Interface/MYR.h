#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> deviceExtensions
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};






struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool allComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class Window
{
public:
    Window(uint32_t width, uint32_t height);
    ~Window();

    void initWindow();

    GLFWwindow* getHandle() const { return window; }

private:
    const uint32_t WIDTH;
    const uint32_t HEIGHT;
    GLFWwindow* window;
};

class Core
{
public:
    Core(bool enableValidationLayers);
    ~Core();

    void initVulkanInstance();
    void initDebugMessenger();
    void initSurface(Window* window);

    VkInstance getInstance() const { return instance; }
    VkSurfaceKHR getSurface() const { return surface; }

private:
    VkDebugUtilsMessengerEXT debugMessenger;
    VkInstance instance;
    VkSurfaceKHR surface;
    bool enableValidationLayers;
};

class Device
{
public:
    Device();
    ~Device();
    
    void setSurface(VkSurfaceKHR surface) { this->surface = surface; }
    void pickPhysicalDevice(VkInstance instance);
    void initLogicalDevice(bool enableValidationLayers);
    SwapChainSupportDetails querySwapChainSupport();
    QueueFamilyIndices findQueueFamilies();
    bool isDeviceSuitable();

    VkDevice getHandle() const { return device; }
    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
    VkQueue getGraphicsQueue() { return graphicsQueue; }
    VkQueue getPresentQueue() { return presentQueue; }
private:
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
};


class SwapChain
{
public:
    SwapChain(Device* device);
    ~SwapChain();

    void initSwapChain(VkSurfaceKHR surface, GLFWwindow* window);
    void initImageViews();

    VkSwapchainKHR getHandle() { return swapChain; }
    VkFormat getImageFormat() { return swapChainImageFormat; };
    uint32_t getImageCount() { return imageCount; }
    VkImageView getView(size_t index) { return swapChainImageViews[index]; }
    VkExtent2D getExtent() { return swapChainExtent; }


private:
    Device* device;
    VkSwapchainKHR swapChain;
    uint32_t imageCount;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
};

class Pipeline
{
public:
    Pipeline(Device* device);
    ~Pipeline();

    void initRenderPass(VkFormat ImageFormat);
    void initGraphicsPipeline();
    void initFramebuffers(SwapChain* swapChain);

    VkPipeline getHandle() { return graphicsPipeline; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkFramebuffer getFramebuffer(uint32_t imageIndex) { return Framebuffers[imageIndex]; }
private:
    Device* device;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> Framebuffers;
};

class Command
{
public:
    Command(Device* device, Pipeline* pipeline);
    ~Command();
    
    void initCommandPool();
    void initCommandBuffer();
    void recordCommandBuffer(uint32_t imageIndex, VkExtent2D extent);

    VkCommandBuffer_T** ofBuffer() { return &commandBuffer; }

private:
    Device* device;
    Pipeline* pipeline;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
};