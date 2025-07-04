#include "MYR.h"
#include <stdexcept>

using namespace MYR;

Command_T::Command_T(Device device, Pipeline pipeline, SwapChain swapChain, const int MAX_FRAMES_IN_FLIGHT) : device(device), pipeline(pipeline), MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT), swapChain(swapChain) {}

Command_T::~Command_T()
{
    vkDestroyCommandPool(device->getHandle(), commandPool, nullptr);
    vkDestroyCommandPool(device->getHandle(), transientCommandPool, nullptr);
}

void Command_T::initCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = device->findQueueFamilies();

    VkCommandPoolCreateInfo cmdpoolInfo{};
    cmdpoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdpoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdpoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device->getHandle(), &cmdpoolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }

    VkCommandPoolCreateInfo tansientpoolInfo{};
    tansientpoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    tansientpoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    tansientpoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device->getHandle(), &tansientpoolInfo, nullptr, &transientCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}
void Command_T::initCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device->getHandle(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
void Command_T::recordCommandBuffer(uint32_t currentFrameIndex, uint32_t imageIndex, VkBuffer viBuffer,uint32_t index_count, std::vector<VkDescriptorSet> *descriptorSets)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrameIndex], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pipeline->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFramebuffer(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[currentFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());

    VkBuffer vertexBuffers[] = { viBuffer };
    VkDeviceSize offsets[] = { sizeof(uint32_t) * index_count };
    vkCmdBindVertexBuffers(commandBuffers[currentFrameIndex], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffers[currentFrameIndex], viBuffer, 0, VK_INDEX_TYPE_UINT32);


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getExtent().width);
    viewport.height = static_cast<float>(swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrameIndex], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain->getExtent();
    vkCmdSetScissor(commandBuffers[currentFrameIndex], 0, 1, &scissor);


    vkCmdBindDescriptorSets(commandBuffers[currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &((*descriptorSets)[currentFrameIndex]), 0, nullptr);

    for (PushConstant& pushConstant: pipeline->getPushConstants())
        vkCmdPushConstants(commandBuffers[currentFrameIndex], pipeline->getPipelineLayout(), pushConstant.stages, pushConstant.offset, pushConstant.size, pushConstant.data);

    vkCmdDrawIndexed(commandBuffers[currentFrameIndex], index_count, 1, 0, 0, 0);


    vkCmdEndRenderPass(commandBuffers[currentFrameIndex]);
    if (vkEndCommandBuffer(commandBuffers[currentFrameIndex]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");

}



VkCommandBuffer Command_T::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = transientCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->getHandle(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Command_T::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->getGraphicsQueue());

    vkFreeCommandBuffers(device->getHandle(), transientCommandPool, 1, &commandBuffer);
}