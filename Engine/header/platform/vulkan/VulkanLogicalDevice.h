#pragma once

#include "vulkan/vulkan_core.h"

namespace FGEngine
{
	class VulkanInstance;
	class VulkanPhysicalDevice;

	class VulkanLogicalDevice
	{
	public:
		VulkanLogicalDevice(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::vector<const char*>& deviceExtensions);
		~VulkanLogicalDevice();

		operator VkDevice () const { return device; }

		VkQueue GetGraphicsQueue() const { return graphicsQueue; }
		VkQueue GetPresentQueue() const { return presentQueue; }

	private:
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
	};
}

