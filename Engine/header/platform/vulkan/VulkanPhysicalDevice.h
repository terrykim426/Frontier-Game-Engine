#pragma once

#include "platform/vulkan/VulkanDefineType.h"

#include "vulkan/vulkan_core.h"

#include <memory>

namespace FGEngine
{
	class VulkanInstance;

	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::vector<const char*>& deviceExtensions);

		void Refresh(const std::shared_ptr<VulkanInstance>& vulkanInstance);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		VkFormat FindDepthFormat() const;

		operator VkPhysicalDevice () const { return physicalDevice; }
		const QueueFamilyIndices GetQueueFamilyIndices() const { return queueFamilyIndices; }
		const SwapChainSupportDetails GetSwapChainSupportDetails() const { return swapChainSupportDetails; }
		VkSampleCountFlagBits GetMaxSampleCount() const { return maxSampleCount; }

	private:
		VkPhysicalDevice physicalDevice;
		QueueFamilyIndices queueFamilyIndices;
		SwapChainSupportDetails swapChainSupportDetails;
		VkSampleCountFlagBits maxSampleCount;
	};
}
