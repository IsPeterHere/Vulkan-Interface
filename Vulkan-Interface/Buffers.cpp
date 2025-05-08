#include "MYR.h"
#include <stdexcept>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
void createBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlagBits info, VkBuffer& buffer, VmaAllocation& allocation);
void copyBuffer(Device* device, VkCommandPool transientCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


Buffers::Buffers(Device* device, Pipeline* pipeline, const int MAX_FRAMES_IN_FLIGHT) : device(device), pipeline(pipeline), MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT){}
Buffers::~Buffers()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        vmaDestroyBuffer(device->getAllocator(), uniformBuffers[i], uniformBuffersAllocation[i]);

    vmaDestroyBuffer(device->getAllocator(), indexBuffer, indexBufferAllocation);
    vmaDestroyBuffer(device->getAllocator(), vertexBuffer, vertexBufferAllocation);
	vkDestroyCommandPool(device->getHandle(), commandPool, nullptr);
    vkDestroyCommandPool(device->getHandle(), transientCommandPool, nullptr);
}


void Buffers::initCommandPool()
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
void Buffers::initCommandBuffers()
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
void Buffers::recordCommandBuffer(uint32_t currentFrameIndex, uint32_t imageIndex, SwapChain* swapChain)
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
    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[currentFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffers[currentFrameIndex], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffers[currentFrameIndex], indexBuffer, 0, VK_INDEX_TYPE_UINT32);


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

    vkCmdDrawIndexed(commandBuffers[currentFrameIndex],index_count, 1, 0, 0, 0);


    vkCmdEndRenderPass(commandBuffers[currentFrameIndex]);
    if (vkEndCommandBuffer(commandBuffers[currentFrameIndex]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
    
}

void Buffers::initVertexBuffer(const std::vector<Vertex> vertices)
{
    vertex_count = static_cast<uint32_t>(vertices.size());


    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT , stagingBuffer, stagingBufferAllocation);

    void* data;
    vmaMapMemory(device->getAllocator(), stagingBufferAllocation, & data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vmaUnmapMemory(device->getAllocator(), stagingBufferAllocation);

    createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, static_cast<VmaAllocationCreateFlagBits>(0), vertexBuffer, vertexBufferAllocation);

    copyBuffer(device,transientCommandPool,stagingBuffer, vertexBuffer, bufferSize);
    vmaDestroyBuffer(device->getAllocator(), stagingBuffer, stagingBufferAllocation);
}

void Buffers::initIndexBuffer(const std::vector<uint32_t> indices)
{
    index_count = static_cast<uint32_t>(indices.size());

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer, stagingBufferAllocation);

    void* data;
    vmaMapMemory(device->getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vmaUnmapMemory(device->getAllocator(), stagingBufferAllocation);

    createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, static_cast<VmaAllocationCreateFlagBits>(0), indexBuffer, indexBufferAllocation);

    copyBuffer(device, transientCommandPool, stagingBuffer, indexBuffer, bufferSize);
    vmaDestroyBuffer(device->getAllocator(), stagingBuffer, stagingBufferAllocation);
}

void Buffers::initUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersAllocation.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, static_cast<VmaAllocationCreateFlagBits>(0), uniformBuffers[i], uniformBuffersAllocation[i]);
        vmaMapMemory(device->getAllocator(), uniformBuffersAllocation[i], &uniformBuffersMapped[i]);
    }
}


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void createBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlagBits info, VkBuffer& buffer, VmaAllocation& allocation) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = info;

    if (vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
    
}

void copyBuffer(Device* device, VkCommandPool transientCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->getGraphicsQueue());

    vkFreeCommandBuffers(device->getHandle(), transientCommandPool, 1, &commandBuffer);
}