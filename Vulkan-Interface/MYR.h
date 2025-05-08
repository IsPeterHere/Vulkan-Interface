#pragma once
#define GLFW_INCLUDE_VULKAN
#ifdef MAIN
#define VMA_IMPLEMENTATION
#endif
#include "vk_mem_alloc.h"
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

struct UniformBufferObject 
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
    void initAllocator(VkInstance);

    VkDevice getHandle() const { return device; }
    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
    VkQueue getGraphicsQueue() { return graphicsQueue; }
    VkQueue getPresentQueue() { return presentQueue; }
    VmaAllocator getAllocator() { return allocator; }
private:
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VmaAllocator allocator;
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
    void initDescriptorSetLayout();
    void initGraphicsPipeline();

    VkPipeline getHandle() { return graphicsPipeline; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkDescriptorSetLayout getDescriptorLayout() { return descriptorSetLayout; }
    VkPipelineLayout getPipelineLayout() { return pipelineLayout; }
private:
    Device* device;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkDescriptorSetLayout descriptorSetLayout;
};

class Buffers
{
public:
    Buffers(Device*, Pipeline*,const int);
    ~Buffers();
    
    void initCommandPool();
    void initCommandBuffers();
    void recordCommandBuffer(uint32_t, uint32_t, SwapChain*);
    void initVertexBuffer(const std::vector<Vertex>);
    void initIndexBuffer(const std::vector<uint32_t>);
    void initUniformBuffers();
    void initDescriptorPool();
    void initDescriptorSets();

    VkCommandBuffer_T** refCommandfBuffer(uint32_t bufferIndex) { return &(commandBuffers[bufferIndex]); }
    void updateUniformBuffer(uint32_t imageIndex, UniformBufferObject ubo) { memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(UniformBufferObject)); }

private:
    const int MAX_FRAMES_IN_FLIGHT;

    Device* device;
    Pipeline* pipeline;

    VkCommandPool commandPool;
    VkCommandPool transientCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VmaAllocation indexBufferAllocation;
    VkBuffer indexBuffer;
    uint32_t index_count;

    VmaAllocation vertexBufferAllocation;
    VkBuffer vertexBuffer;
    uint32_t vertex_count;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VmaAllocation> uniformBuffersAllocation;
    std::vector<void*> uniformBuffersMapped;

};