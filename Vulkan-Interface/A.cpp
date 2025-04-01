#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MYR.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint> 
#include <limits> 
#include <algorithm>
#include <fstream>

//List of required extensions


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
        pipeline(new Pipeline(device))
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

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

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

        vkDestroyCommandPool(device->getHandle(), commandPool, nullptr);

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

        device->pickPhysicalDevice(core->getInstance(), core->getSurface());
        device->initLogicalDevice(enableValidationLayers, core->getSurface());

        swapChain->initSwapChain(core->getSurface(),window->getHandle());
        swapChain->initImageViews();

        pipeline->initRenderPass(swapChain->getImageFormat());
        pipeline->initGraphicsPipeline();
        pipeline->initFramebuffers(swapChain);

        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    }

   
    void createCommandPool() 
    {
        QueueFamilyIndices queueFamilyIndices = device->findQueueFamilies(core->getSurface());

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device->getHandle(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    void createCommandBuffer() 
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device->getHandle(), &allocInfo, &commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pipeline->getRenderPass();
        renderPassInfo.framebuffer = pipeline->getFramebuffer(imageIndex);
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain->getExtent();
        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChain->getExtent().width);
            viewport.height = static_cast<float>(swapChain->getExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = swapChain->getExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to record command buffer!");
        }
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

        vkResetCommandBuffer(commandBuffer, 0);
        recordCommandBuffer(commandBuffer, imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
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