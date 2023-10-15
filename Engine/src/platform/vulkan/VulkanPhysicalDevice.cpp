#include "pch.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"
#include "platform/vulkan/VulkanInstance.h"

#include <algorithm>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FGEngine
{
#pragma region Utilities
	std::vector<VkPhysicalDevice> GetAllPhysicalDevices(VkInstance vulkanInstance)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);

		Check(deviceCount > 0, "No vulkan-supported GPU is found!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

		return devices;
	}

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices queueFamilyIndices;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

		int i = 0;
		for (const VkQueueFamilyProperties& property : properties)
		{
			if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueFamilyIndices.graphicsFamily = i;
			}

			VkBool32 bHasPresentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &bHasPresentSupport);
			if (bHasPresentSupport)
			{
				queueFamilyIndices.presentFamily = i;
			}

			if (queueFamilyIndices.IsComplete())
			{
				break;
			}
			i++;
		}

		return queueFamilyIndices;
	}

	bool IsDeviceExtensionsSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// check that all required extensions is available
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const VkExtensionProperties& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
			if (requiredExtensions.empty())
			{
				break;
			}
		}

		return requiredExtensions.empty();
	}

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails supportDetails;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &supportDetails.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			supportDetails.surfaceFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, supportDetails.surfaceFormats.data());
		}

		uint32_t modeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);
		if (modeCount != 0)
		{
			supportDetails.presentModes.resize(modeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &formatCount, supportDetails.presentModes.data());
		}

		return supportDetails;
	}

	bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions)
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device, surface);

		bool bExtensionSupported = IsDeviceExtensionsSupported(device, deviceExtensions);

		bool bSwapChainAdequate = false;
		if (bExtensionSupported)
		{
			SwapChainSupportDetails supportDetails = QuerySwapChainSupport(device, surface);
			bSwapChainAdequate = !supportDetails.surfaceFormats.empty() && !supportDetails.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return queueFamilyIndices.IsComplete()
			&& bExtensionSupported
			&& bSwapChainAdequate
			&& supportedFeatures.samplerAnisotropy;
	}

	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		VkSampleCountFlags counts =
			physicalDeviceProperties.limits.framebufferColorSampleCounts &
			physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkPhysicalDevice PickPhysicalDevice(std::vector<VkPhysicalDevice>& devices, const VulkanInstance* vulkanInstance, const std::vector<const char*>& deviceExtensions)
	{
		if (devices.size() == 1)
		{
			return devices[0];
		}
		else if (devices.size() > 1)
		{
			// sort by scoring the supported properties and features
			auto DeviceScore = [&](VkPhysicalDevice device)
			{
				if (!IsDeviceSuitable(device, vulkanInstance->GetSurface(), deviceExtensions))
				{
					return -1;
				}

				int score = 0;
				VkPhysicalDeviceProperties properties;
				VkPhysicalDeviceFeatures features;
				vkGetPhysicalDeviceProperties(device, &properties);
				vkGetPhysicalDeviceFeatures(device, &features);

				// discrete gpu has performance advantage
				if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					score += 1000;
				}

				// maximum possible size of textures affects graphics quality
				score += properties.limits.maxImageDimension2D;

				/* uncomment this if geometry shader support is required
				if (!features.geometryShader)
				{
					score = 0;
				}
				*/

				return score;
			};

			std::sort(devices.begin(), devices.end(),
				[&](auto A, auto B)
				{
					return DeviceScore(A) > DeviceScore(B);
				});

			return devices[0];
		}

		NoEntry("Failed to find suitable GPU!");
		return nullptr;
	}
#pragma endregion


	VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanInstance* vulkanInstance, const std::vector<const char*>& deviceExtensions)
	{
		std::vector<VkPhysicalDevice> devices = GetAllPhysicalDevices(*vulkanInstance);
		physicalDevice = PickPhysicalDevice(devices, vulkanInstance, deviceExtensions);

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		LogInfo("Device Name: %s", properties.deviceName);

		swapChainSupportDetails = QuerySwapChainSupport(physicalDevice, vulkanInstance->GetSurface());
		queueFamilyIndices = FindQueueFamilies(physicalDevice, vulkanInstance->GetSurface());
		maxSampleCount = GetMaxUsableSampleCount(physicalDevice);
	}
}