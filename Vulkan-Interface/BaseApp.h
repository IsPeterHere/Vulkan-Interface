#pragma once
#define MAIN
#include "MYR.h"
#include "Camera.h"
#include "Control.h"
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

const bool enableValidationLayers{ true };

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class BaseApp
{
public:

    const int MAX_FRAMES_IN_FLIGHT{ 2 };

    std::vector<MYR::Vertex> vertices{};
    std::vector<uint32_t> indices{};


    BaseApp(uint32_t width, uint32_t height) :
        window(new MYR::Window_T(width, height)),
        core(new MYR::Core_T(enableValidationLayers)),
        device(new MYR::Device_T()),
        swapChain(new MYR::SwapChain_T(device)),
        pipeline(new MYR::Pipeline_T(device)),
        command(new MYR::Command_T(device, pipeline, swapChain, MAX_FRAMES_IN_FLIGHT)),
        syncManager(new MYR::SyncManager_T(device)),
        imageManager(new MYR::ImageManager_T(device, command)),
        bufferManager(new MYR::BufferManager_T(device, command)),
        buffers(new MYR::Buffers_T(device, pipeline, command, MAX_FRAMES_IN_FLIGHT)),
        camera(new Camera())
    {}
    ~BaseApp() { cleanup(); }

    void initComponents()
    {
        window->initWindow();
        initVulkan();
        control = Control::makeControl(window->getHandle());
    }

    void run()
    {
        while (!glfwWindowShouldClose(window->getHandle()))
        {
            glfwPollEvents();
            control->update_camera(camera, 20, 5);
            doFrame();
        }
    }

    void run_with_update_function(bool (*f)(double delta_T), float f_call_time)
    {
        while (!glfwWindowShouldClose(window->getHandle()))
        {
            static std::chrono::steady_clock::time_point  startTime = std::chrono::high_resolution_clock::now();
            static float f_call_time_ellapsed{ 0 };

            std::chrono::steady_clock::time_point  currentTime = std::chrono::high_resolution_clock::now();
            f_call_time_ellapsed = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            if (f_call_time_ellapsed >= f_call_time)
            {
                f_call_time_ellapsed = 0;
                startTime = std::chrono::high_resolution_clock::now();
                bool changes_made{ f(f_call_time) };
                if (changes_made) flush_mesh_update();
            }

            glfwPollEvents();
            control->update_camera(camera, 20, 5);
            doFrame();
        }
    }

    void flush_mesh_update()
    {
        vkWaitForFences(device->getHandle(), 1, &inFlightFences[(currentFrame + 1) % MAX_FRAMES_IN_FLIGHT], VK_TRUE, UINT64_MAX);
        if (buffers->getVIBuffer() != NULL)
            bufferManager->destroyBuffer(buffers->getVIBuffer());
        buffers->initVIBuffer(bufferManager, vertices, indices);
    }
    VkExtent2D getWindowExtent() { return swapChain->getExtent(); }
    void close_window() { window->close_window(); }

    void createPushConstant(MYR::PushConstant p)
    {
        pipeline->addPushConstant(p);
    }

    Camera* camera;
    Control* control;
private:

    MYR::Window window;
    MYR::Core core;
    MYR::Device device;
    MYR::SwapChain swapChain;
    MYR::Pipeline pipeline;
    MYR::Command command;
    MYR::SyncManager syncManager;
    MYR::ImageManager imageManager;
    MYR::BufferManager bufferManager;
    MYR::Buffers buffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0;
    bool drawing{ true };


    void cleanup()
    {
        vkDeviceWaitIdle(device->getHandle());

        delete swapChain;
        Control::destroyControl();
        delete camera;
        delete imageManager;
        delete bufferManager;
        delete syncManager;
        delete buffers;
        delete command;
        delete pipeline;
        delete device;
        delete core;
        delete window;
    }

    void initVulkan()
    {
        core->initVulkanInstance();
        core->initDebugMessenger();
        core->initSurface(window);

        device->setSurface(core->getSurface());

        device->pickPhysicalDevice(core->getInstance());
        device->initLogicalDevice(enableValidationLayers);
        device->initAllocator(core->getInstance());

        swapChain->initSwapChain(core->getSurface(), window->getHandle());
        swapChain->initImageViews();

        pipeline->initRenderPass(swapChain->getImageFormat());
        pipeline->initDescriptorSetLayout();
        pipeline->initGraphicsPipeline();


        command->initCommandPool();
        command->initCommandBuffers();

        swapChain->initDepthStencil(imageManager);
        swapChain->initFramebuffers(pipeline->getRenderPass());

        buffers->initDescriptorPool();
        buffers->initUniformBuffers(bufferManager, sizeof(UniformBufferObject));
        buffers->initDescriptorSets();

        createSyncObjects();
    }


    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            imageAvailableSemaphores[i] = syncManager->createSemaphore();
            renderFinishedSemaphores[i] = syncManager->createSemaphore();
            inFlightFences[i] = syncManager->createFence();
        }
    }

    void doFrame()
    {
        vkWaitForFences(device->getHandle(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device->getHandle(), swapChain->getHandle(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");


        vkResetFences(device->getHandle(), 1, &inFlightFences[currentFrame]);

        updateUniformBuffer(currentFrame);
        if (drawing) drawFrame(imageIndex);
        
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void drawFrame(uint32_t imageIndex)
    {
        vkResetCommandBuffer(*(command->refCommandfBuffer(currentFrame)), 0);
        command->recordCommandBuffer(currentFrame, imageIndex, buffers->getVIBuffer(), buffers->getIndexCount(), buffers->getDiscriptorSets());


        std::vector<VkSemaphore> signalSemaphores = { renderFinishedSemaphores[currentFrame] };
        command->submitCommandBuffer(currentFrame,imageIndex, imageAvailableSemaphores[currentFrame], signalSemaphores, inFlightFences[currentFrame]);

        VkResult result{ swapChain->presentImage(imageIndex, signalSemaphores) };

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        UniformBufferObject ubo{};
        ubo.model = ubo.model = glm::mat4(1.0f);
        ubo.view = camera->get_look_at();
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChain->getExtent().width / (float)swapChain->getExtent().height, 0.1f, 100.0f);
        ubo.proj[1][1] *= -1; //GLM originally designed for OpenGL, where the y coordinate is inverted.

        buffers->updateUniformBuffer(currentImage, &ubo, sizeof(UniformBufferObject));
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window->getHandle(), &width, &height);
        if (width == 0 || height == 0) 
            drawing = false;  
        else
        {
            drawing = true;
            vkDeviceWaitIdle(device->getHandle());

            delete swapChain;
            swapChain = new MYR::SwapChain_T(device);
            command->set_swapChain(swapChain);

            swapChain->initSwapChain(core->getSurface(), window->getHandle());
            swapChain->initImageViews();
            swapChain->initDepthStencil(imageManager);
            swapChain->initFramebuffers(pipeline->getRenderPass());
        }

    }
};