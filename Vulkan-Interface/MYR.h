#pragma once
#define GLFW_INCLUDE_VULKAN
#ifdef MAIN
#define VMA_IMPLEMENTATION
#endif
#include "vk_mem_alloc.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>
#include <stdexcept>
#include<unordered_map>
#include<unordered_set>



namespace MYR
{
    typedef class ImageManager_T* ImageManager;
    typedef class BufferManager_T* BufferManager;
    typedef class Window_T* Window;
    typedef class Core_T* Core;
    typedef class Device_T* Device;
    typedef class SwapChain_T* SwapChain;
    typedef class Pipeline_T* Pipeline;
    typedef class Command_T* Command;
    typedef class Buffers_T* Buffers;

    const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };

    const std::vector<const char*> deviceExtensions
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
    };

    struct PushConstant
    {
        uint16_t offset;
        uint16_t size;
        void* data;
        VkShaderStageFlags stages;
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

    class ImageManager_T;

    class Window_T
    {
    public:
        bool windowResized = false;

        Window_T(uint32_t width, uint32_t height);
        ~Window_T();

        void initWindow();

        GLFWwindow* getHandle() const { return window; }

    private:
        const uint32_t WIDTH;
        const uint32_t HEIGHT;
        GLFWwindow* window;
    };

    class Core_T
    {
    public:
        Core_T(bool enableValidationLayers);
        ~Core_T();

        void initVulkanInstance();
        void initDebugMessenger();
        void initSurface(Window window);

        VkInstance getInstance() const { return instance; }
        VkSurfaceKHR getSurface() const { return surface; }

    private:
        VkDebugUtilsMessengerEXT debugMessenger;
        VkInstance instance;
        VkSurfaceKHR surface;
        bool enableValidationLayers;
    };

    class Device_T
    {
    public:
        Device_T();
        ~Device_T();

        void setSurface(VkSurfaceKHR surface) { this->surface = surface; }
        void pickPhysicalDevice(VkInstance);
        void initLogicalDevice(bool);
        SwapChainSupportDetails querySwapChainSupport();
        QueueFamilyIndices findQueueFamilies();
        bool isDeviceSuitable();
        VkFormat findDepthFormat();

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


    class SwapChain_T
    {
    public:
        SwapChain_T(Device);
        ~SwapChain_T();

        void initSwapChain(VkSurfaceKHR, GLFWwindow*);
        void initImageViews();
        void initDepthStencil(ImageManager);
        void initFramebuffers(VkRenderPass);


        VkSwapchainKHR getHandle() { return swapChain; }
        VkFormat getImageFormat() { return swapChainImageFormat; };
        uint32_t getImageCount() { return imageCount; }
        VkImageView getView(size_t index) { return swapChainImageViews[index]; }
        VkExtent2D getExtent() { return swapChainExtent; }
        VkFramebuffer getFramebuffer(uint32_t imageIndex) { return Framebuffers[imageIndex]; }

    private:
        Device device;

        VkSwapchainKHR swapChain;
        uint32_t imageCount;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> Framebuffers;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        VkImage depthImage;
        VkImageView depthImageView;
    };

    class Pipeline_T
    {
    public:
        Pipeline_T(Device);
        ~Pipeline_T();

        void initRenderPass(VkFormat);
        void initDescriptorSetLayout();
        void initGraphicsPipeline();
        void addPushConstant(PushConstant);

        VkPipeline getHandle() { return graphicsPipeline; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkDescriptorSetLayout getDescriptorLayout() { return descriptorSetLayout; }
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

        std::vector<PushConstant>& getPushConstants() { return pushConstants; }

    private:
        Device device;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkDescriptorSetLayout descriptorSetLayout;

        std::vector<PushConstant> pushConstants;
        std::vector<VkPushConstantRange> pushConstantRanges;

    };


    class Command_T
    {
    public:
        Command_T(Device, Pipeline, SwapChain, const int);
        ~Command_T();

        void initCommandPool();
        void initCommandBuffers();
        void recordCommandBuffer(uint32_t, uint32_t, VkBuffer, uint32_t, std::vector<VkDescriptorSet>*);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        VkCommandBuffer_T** refCommandfBuffer(uint32_t bufferIndex) { return &(commandBuffers[bufferIndex]); }
        VkCommandPool getTransientCommandPool() { return transientCommandPool; }

    private:
        const int MAX_FRAMES_IN_FLIGHT;
        Device device;
        Pipeline pipeline;
        SwapChain swapChain;

        VkCommandPool commandPool;
        VkCommandPool transientCommandPool;
        std::vector<VkCommandBuffer> commandBuffers;
    };


    class ImageManager_T
    {
    public:
        ImageManager_T(Device, Command);
        ~ImageManager_T();


        void createImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage*);
        VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool stencilComponent = false);

    private:
        Device device;
        Command command;

        std::unordered_map<VkImage, VmaAllocation> allocations{};
    };

    class BufferManager_T
    {
    public:
        BufferManager_T(Device, Command);
        ~BufferManager_T();

        void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VmaAllocationCreateFlags, VkBuffer*);
        void destroyBuffer(VkBuffer);
        void copyBuffer(VkBuffer, VkBuffer, uint32_t, VkDeviceSize);
        void mapMemory(VkBuffer, void**);
        void unmapMemory(VkBuffer);

    private:
        Device device;
        Command command;

        std::unordered_map<VkBuffer, VmaAllocation> allocations{};
        std::unordered_set<VkBuffer> mappedBuffers{};

    };
    class Buffers_T
    {
    public:
        Buffers_T(Device, Pipeline, Command, const int);
        ~Buffers_T();

        void initVIBuffer(BufferManager bufferManager, const std::vector<Vertex>, const std::vector<uint32_t>);
        void initUniformBuffers(BufferManager bufferManager, size_t);
        void initDescriptorPool();
        void initDescriptorSets();

        VkBuffer getVIBuffer() { return viBuffer; }

        void updateUniformBuffer(uint32_t imageIndex, void* ubo, size_t uboSize) { memcpy(uniformBuffersMapped[imageIndex], ubo, uboSize); }

        uint32_t getIndexCount() { return index_count; }
        std::vector<VkDescriptorSet>* getDiscriptorSets() { return &descriptorSets; }

    private:
        const int MAX_FRAMES_IN_FLIGHT;
        Device device;
        Pipeline pipeline;
        Command command;

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        VmaAllocation viBufferAllocation;
        VkBuffer viBuffer;
        uint32_t index_count;
        uint32_t vertex_count;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<void*> uniformBuffersMapped;

    };

}