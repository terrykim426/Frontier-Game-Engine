#pragma once

#include "platform/vulkan/VulkanDefineType.h"

struct VkPhysicalDevice_T;

namespace FGEngine
{
	class VulkanInstance;

	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(const VulkanInstance* vulkanInstance, const std::vector<const char*>& deviceExtensions);

		void Refresh(const VulkanInstance* vulkanInstance);

		operator VkPhysicalDevice_T* () const { return physicalDevice; }
		const QueueFamilyIndices GetQueueFamilyIndices() const { return queueFamilyIndices; }
		const SwapChainSupportDetails GetSwapChainSupportDetails() const { return swapChainSupportDetails; }
		VkSampleCountFlagBits GetMaxSampleCount() const { return maxSampleCount; }

	private:
		VkPhysicalDevice_T* physicalDevice;
		QueueFamilyIndices queueFamilyIndices;
		SwapChainSupportDetails swapChainSupportDetails;
		VkSampleCountFlagBits maxSampleCount;
	};
}
