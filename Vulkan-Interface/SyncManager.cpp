#include "MYR.h"

using namespace MYR;

SyncManager_T::SyncManager_T(Device device) : device(device){}
SyncManager_T::~SyncManager_T()
{
    for (size_t i = 0; i < semaphores.size(); i++)
    {
        vkDestroySemaphore(device->getHandle(), semaphores[i], nullptr);
    }

    for (size_t i = 0; i < fences.size(); i++)
    {
        vkDestroyFence(device->getHandle(), fences[i], nullptr);
    }
}

VkSemaphore SyncManager_T::createSemaphore()
{
    semaphores.resize(semaphores.size() + 1);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &semaphores[semaphores.size()-1]) != VK_SUCCESS)
        throw std::runtime_error("failed to create Semaphore object!");
    return semaphores[semaphores.size()-1];
}

VkFence SyncManager_T::createFence()
{
    fences.resize(fences.size() + 1);
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(device->getHandle(), &fenceInfo, nullptr, &fences[fences.size()-1]) != VK_SUCCESS)
        throw std::runtime_error("failed to create Fence object!");
    return fences[fences.size()-1];
}