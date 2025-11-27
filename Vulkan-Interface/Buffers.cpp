#include "MYR.h"

using namespace MYR;

Buffers_T::Buffers_T(Device device, Pipeline pipeline,Command command, const int MAX_FRAMES_IN_FLIGHT) : device(device), pipeline(pipeline), MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT), command(command){}
Buffers_T::~Buffers_T()
{
    vkDestroyDescriptorPool(device->getHandle(), descriptorPool, nullptr);
}

void Buffers_T::createVIBuffer(BufferManager bufferManager,const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    vertex_count = static_cast<uint32_t>(vertices.size());
    index_count = static_cast<uint32_t>(indices.size());

    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
    VkDeviceSize viBufferSize = vertexBufferSize + indexBufferSize;

    VkBuffer stagingBuffer;

    bufferManager->createBuffer(viBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, static_cast<VmaAllocationCreateFlagBits>(0), &viBuffer);


    bufferManager->createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, &stagingBuffer);
    void* idata;
    bufferManager->mapMemory(stagingBuffer, &idata);
    memcpy(idata, indices.data(), (size_t)indexBufferSize);
    bufferManager->unmapMemory(stagingBuffer);
    bufferManager->copyBuffer(stagingBuffer, viBuffer,0, indexBufferSize);
    bufferManager->destroyBuffer(stagingBuffer);

    bufferManager->createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, &stagingBuffer);
    void* vdata;
    bufferManager->mapMemory(stagingBuffer, &vdata);
    memcpy(vdata, vertices.data(), (size_t)vertexBufferSize);
    bufferManager->unmapMemory(stagingBuffer);
    bufferManager->copyBuffer(stagingBuffer, viBuffer,sizeof(indices[0])*index_count, vertexBufferSize);
    bufferManager->destroyBuffer(stagingBuffer);
}

void Buffers_T::initUniformBuffers(BufferManager bufferManager,size_t uboSize)
{
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        bufferManager->createBuffer(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, &uniformBuffers[i]);
        bufferManager->mapMemory(uniformBuffers[i], &uniformBuffersMapped[i]);
    }
}

void Buffers_T::initDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device->getHandle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");
    
}

void Buffers_T::initDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline->getDescriptorLayout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device->getHandle(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<VkWriteDescriptorSet > descriptorWriters{ 1 };

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWriters[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWriters[0].dstSet = descriptorSets[i];
        descriptorWriters[0].dstBinding = 0;
        descriptorWriters[0].dstArrayElement = 0;
        descriptorWriters[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWriters[0].descriptorCount = 1;
        descriptorWriters[0].pBufferInfo = &bufferInfo;
        descriptorWriters[0].pImageInfo = nullptr;
        descriptorWriters[0].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device->getHandle(), (uint32_t) descriptorWriters.size(), descriptorWriters.data(), 0, nullptr);
    }
    
}

