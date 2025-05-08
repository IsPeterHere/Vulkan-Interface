#define MAIN
#include "MYR.h"
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


const bool enableValidationLayers{true};


class HelloTriangleApplication 
{
public:

    const uint32_t WIDTH{ 800 };
    const uint32_t HEIGHT{ 600 };
    const int MAX_FRAMES_IN_FLIGHT{ 2 };

    const std::vector<Vertex> vertices = 
    {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint32_t> indices = 
    {
    0, 1, 2, 2, 3, 0
    };


    HelloTriangleApplication() : 
        window(new Window(WIDTH,HEIGHT)),
        core(new Core(enableValidationLayers)), 
        device(new Device()),
        swapChain(new SwapChain(device)),
        pipeline(new Pipeline(device)),
        buffers(new Buffers(device,pipeline, MAX_FRAMES_IN_FLIGHT))
    {
    }

    void run() 
    {
        window->initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window->getHandle(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window->getHandle(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device->getHandle());

        delete swapChain;
        swapChain = new SwapChain(device);

        swapChain->initSwapChain(core->getSurface(), window->getHandle());
        swapChain->initImageViews();
        swapChain->initFramebuffers(pipeline->getRenderPass());

    }

private:

    Window *window;
    Core* core;
    Device* device;
    SwapChain* swapChain;
    Pipeline* pipeline;
    Buffers* buffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0;

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window->getHandle())) 
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device->getHandle());
    }
    void cleanup()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(device->getHandle(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device->getHandle(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device->getHandle(), inFlightFences[i], nullptr);
        }

        delete buffers;
        delete pipeline;
        delete swapChain;
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

        swapChain->initSwapChain(core->getSurface(),window->getHandle());
        swapChain->initImageViews();

        pipeline->initRenderPass(swapChain->getImageFormat());
        pipeline->initDescriptorSetLayout();
        pipeline->initGraphicsPipeline();

        swapChain->initFramebuffers(pipeline->getRenderPass());

        buffers->initCommandPool();
        buffers->initVertexBuffer(vertices);
        buffers->initIndexBuffer(indices);
        buffers->initUniformBuffers();
        buffers->initCommandBuffers();
        

        createSyncObjects();
    }


    void createSyncObjects() 
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            if (vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->getHandle(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void drawFrame()
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

        vkResetCommandBuffer(*(buffers->refCommandfBuffer(currentFrame)), 0);
        buffers->recordCommandBuffer(currentFrame,imageIndex,swapChain);

        updateUniformBuffer(currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers->refCommandfBuffer(currentFrame);
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) 
            throw std::runtime_error("failed to submit draw command buffer!");


        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { swapChain->getHandle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(device->getGraphicsQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->windowResized) 
        {
            window->windowResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) 
            throw std::runtime_error("failed to present swap chain image!");
 

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void updateUniformBuffer(uint32_t currentImage) 
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChain->getExtent().width / (float) swapChain->getExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1; //GLM originally designed for OpenGL, where the y coordinate is inverted.

        buffers->updateUniformBuffer(currentImage, ubo);
    }
};

int main() 
{
    try 
    {
        HelloTriangleApplication app;
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}