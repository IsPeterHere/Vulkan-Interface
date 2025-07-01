#include "MYR.h"


BufferManager::BufferManager(Device* device, Command* command) : device(device), command(command) {}
BufferManager::~BufferManager()
{

    assert(allocations.size() == buffers.size() && "Buffer Count does not Match Allocation count");
    for (int i {0} ; i< allocations.size(); i++)
        vmaDestroyBuffer(device->getAllocator(), buffers[i], allocations[i]);
}


void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlags info, VkBuffer& buffer) 
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = info;

    allocations.reserve(allocations.size() + 1);
    buffers.push_back(buffer);

    if (vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &buffer, &allocations.back(), nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");

}

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t dst_offset, VkDeviceSize size) 
{
    VkCommandBuffer commandBuffer = command->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = dst_offset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    command->endSingleTimeCommands(commandBuffer);
}
