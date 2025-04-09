#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <glm/glm.hpp>
#include <array>


const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> deviceExtensions
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};



struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
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
    bool windowResized = false;

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
    void pickPhysicalDevice(VkInstance);
    void initLogicalDevice(bool);
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
    SwapChain(Device*);
    ~SwapChain();

    void initSwapChain(VkSurfaceKHR, GLFWwindow*);
    void initImageViews();
    void initFramebuffers(VkRenderPass renderPass);


    VkSwapchainKHR getHandle() { return swapChain; }
    VkFormat getImageFormat() { return swapChainImageFormat; };
    uint32_t getImageCount() { return imageCount; }
    VkImageView getView(size_t index) { return swapChainImageViews[index]; }
    VkExtent2D getExtent() { return swapChainExtent; }
    VkFramebuffer getFramebuffer(uint32_t imageIndex) { return Framebuffers[imageIndex]; }

private:
    Device* device;

    VkSwapchainKHR swapChain;
    uint32_t imageCount;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> Framebuffers;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
};

class Pipeline
{
public:
    Pipeline(Device*);
    ~Pipeline();

    void initRenderPass(VkFormat);
    void initGraphicsPipeline();

    VkPipeline getHandle() { return graphicsPipeline; }
    VkRenderPass getRenderPass() { return renderPass; }
private:
    Device* device;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
};

class Buffers
{
public:
    Buffers(Device*, Pipeline*);
    ~Buffers();
    
    void initCommandPool();
    void initCommandBuffers(const int);
    void recordCommandBuffer(uint32_t, uint32_t, SwapChain* swapChain);
    void initVertexBuffer(const std::vector<Vertex>);

    VkCommandBuffer_T** refCommandfBuffer(uint32_t bufferIndex) { return &(commandBuffers[bufferIndex]); }

private:
    Device* device;
    Pipeline* pipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertex_count;
};