#include "MYR.h"

using namespace MYR;

BufferManager_T::BufferManager_T(Device device, Command command) : device(device), command(command) {}
BufferManager_T::~BufferManager_T()
{
    for (auto& buffer : mappedBuffers)
        vmaUnmapMemory(device->getAllocator(), allocations[buffer]);

    for (auto& kv : allocations)
        vmaDestroyBuffer(device->getAllocator(), kv.first, kv.second);
}


void BufferManager_T::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlags info, VkBuffer* buffer)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = info;

    VmaAllocation new_allocation {};
    if (vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, buffer, &new_allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");

    allocations[*buffer] = std::move(new_allocation);
}

void BufferManager_T::destroyBuffer(VkBuffer buffer)
{
    vmaDestroyBuffer(device->getAllocator(), buffer, allocations[buffer]);
    allocations.erase(buffer);
}

void BufferManager_T::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t dst_offset, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = command->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = dst_offset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    command->endSingleTimeCommands(commandBuffer);
}


void BufferManager_T::mapMemory(VkBuffer buffer, void** data)
{
    vmaMapMemory(device->getAllocator(), allocations[buffer], data);
    mappedBuffers.insert(buffer);
}
void BufferManager_T::unmapMemory(VkBuffer buffer)
{
    vmaUnmapMemory(device->getAllocator(), allocations[buffer]);
    mappedBuffers.erase(buffer);
}