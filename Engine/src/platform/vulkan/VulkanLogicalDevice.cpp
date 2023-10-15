#include "pch.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "platform/vulkan/VulkanInstance.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"

#include <set>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FGEngine
{
	VulkanLogicalDevice::VulkanLogicalDevice(const VulkanInstance* vulkanInstance, const VulkanPhysicalDevice* physicalDevice, const std::vector<const char*>& deviceExtensions)
	{
		QueueFamilyIndices queueFamilyIndices = physicalDevice->GetQueueFamilyIndices();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilyIndices
		{
			queueFamilyIndices.graphicsFamily.value(),
				queueFamilyIndices.presentFamily.value(),
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndices)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo  createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

		if (vulkanInstance->bEnableValidationLayers)
		{
			createInfo.ppEnabledLayerNames = vulkanInstance->validationLayers.data();
			createInfo.enabledLayerCount = static_cast<uint32_t>(vulkanInstance->validationLayers.size());
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device);
		Check(result == VK_SUCCESS, "Failed to create logical device. Vulkan error: %d", result);

		vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	}

	VulkanLogicalDevice::~VulkanLogicalDevice()
	{
		vkDestroyDevice(device, nullptr);
	}
}
