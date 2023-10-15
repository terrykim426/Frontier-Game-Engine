#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FGEngine
{
	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice(VkInstance vulkanInstance);

	private:
		std::vector<VkPhysicalDevice> GetAllPhysicalDevices(VkInstance vulkanInstance);

		bool IsDeviceSuitable(VkPhysicalDevice device);

	private:
		VkPhysicalDevice physicalDevice;
	};
}
