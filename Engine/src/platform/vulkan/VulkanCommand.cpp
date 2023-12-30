#include "pch.h"
#include "platform/vulkan/VulkanCommand.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"
#include "platform/vulkan/VulkanLogicalDevice.h"

namespace FGEngine
{
	VulkanCommand::VulkanCommand(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, uint32_t bufferCount)
	{
		logicalDevice = inLogicalDevice;

		CreateCommandPool(physicalDevice, logicalDevice);
		CreateCommandBuffers(logicalDevice, bufferCount);
	}

	VulkanCommand::~VulkanCommand()
	{
		vkDestroyCommandPool(*logicalDevice, commandPool, nullptr);
	}

	void VulkanCommand::CreateCommandPool(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& logicalDevice)
	{
		QueueFamilyIndices queueFamilyIndices = physicalDevice->GetQueueFamilyIndices();

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		VkResult result = vkCreateCommandPool(*logicalDevice, &createInfo, nullptr, &commandPool);
		Check(result == VK_SUCCESS, "Failed to create command pool. Vulkan error: %d", result);
	}

	void VulkanCommand::CreateCommandBuffers(const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, uint32_t bufferCount)
	{
		commandBuffers.resize(bufferCount);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(*logicalDevice, &allocateInfo, commandBuffers.data());
		Check(result == VK_SUCCESS, "Failed to allocate command buffers. Vulkan error: %d", result);
	}
}
