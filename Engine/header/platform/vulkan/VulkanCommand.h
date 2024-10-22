#pragma once

#include "vulkan/vulkan_core.h"

#include <memory>

namespace FGEngine
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;

	class VulkanCommand
	{
	public:
		VulkanCommand(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, uint32_t bufferCount);
		~VulkanCommand();

	private:
		void CreateCommandPool(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& logicalDevice);
		void CreateCommandBuffers(const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, uint32_t bufferCount);

	public:
		VkCommandPool GetPool() const { return commandPool; }
		bool GetBuffer(uint32_t index, VkCommandBuffer& OutCommandBuffer) const
		{
			if (index < 0 || index >= commandBuffers.size()) return false;
			OutCommandBuffer = commandBuffers[index];
			return true;
		}

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;
	};
}

