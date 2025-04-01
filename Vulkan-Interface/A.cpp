#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MYR.h"
#include <iostream>
#include <stdexcept>



const bool enableValidationLayers{true};


class HelloTriangleApplication 
{
public:

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    HelloTriangleApplication() : 
        window(new Window(WIDTH,HEIGHT)),
        core(new Core(enableValidationLayers)), 
        device(new Device()),
        swapChain(new SwapChain(device)),
        pipeline(new Pipeline(device)),
        command(new Command(device,pipeline))
    {
    }

    void run() 
    {
        window->initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:


    Window *window;
    Core* core;
    Device* device;
    SwapChain* swapChain;
    Pipeline* pipeline;
    Command* command;

    

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window->getHandle())) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device->getHandle());
    }
    void cleanup()
    {
        vkDestroySemaphore(device->getHandle(), renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device->getHandle(), imageAvailableSemaphore, nullptr);
        vkDestroyFence(device->getHandle(), inFlightFence, nullptr);

        delete command;
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

        swapChain->initSwapChain(core->getSurface(),window->getHandle());
        swapChain->initImageViews();

        pipeline->initRenderPass(swapChain->getImageFormat());
        pipeline->initGraphicsPipeline();
        pipeline->initFramebuffers(swapChain);

        command->initCommandPool();
        command->initCommandBuffer();

        createSyncObjects();
    }


    void createSyncObjects() 
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device->getHandle(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create Sync Objects!");
        }
    }

    void drawFrame()
    {
        vkWaitForFences(device->getHandle(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device->getHandle(), 1, &inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device->getHandle(), swapChain->getHandle(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(*(command->ofBuffer()), 0);
        command->recordCommandBuffer(imageIndex,swapChain->getExtent());

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = command->ofBuffer();
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { swapChain->getHandle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(device->getGraphicsQueue(), &presentInfo);
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