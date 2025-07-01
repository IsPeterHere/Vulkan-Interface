#include "MYR.h"


BufferManager::BufferManager(Device* device, Command* command) : device(device), command(command) {}
BufferManager::~BufferManager()
{
    for (auto& buffer : mappedBuffers)
        vmaUnmapMemory(device->getAllocator(), allocations[buffer]);

    for (auto& kv : allocations)
        vmaDestroyBuffer(device->getAllocator(), kv.first, kv.second);
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

    allocations[buffer] = NULL;

    if (vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &buffer, &(allocations[buffer]), nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");

}

void BufferManager::destroyBuffer(VkBuffer& buffer)
{
    vmaDestroyBuffer(device->getAllocator(), buffer, allocations[buffer]);
    allocations.erase(buffer);
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


void BufferManager::mapMemory(VkBuffer &buffer, void** data)
{
    std::cout << "CCC" << device->getAllocator();
    std::cout << "AAA" << allocations[buffer];
    std::cout << "MMM" << data;
    vmaMapMemory(device->getAllocator(), allocations[buffer], data);
    mappedBuffers.insert(buffer);
}
void BufferManager::unmapMemory(VkBuffer &buffer)
{
    vmaUnmapMemory(device->getAllocator(), allocations[buffer]);
    mappedBuffers.erase(buffer);
}