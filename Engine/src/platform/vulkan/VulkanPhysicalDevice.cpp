#include "pch.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"


namespace FGEngine
{
	VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance vulkanInstance)
	{
		//std::vector<VkPhysicalDevice> devices = GetAllPhysicalDevices(vulkanInstance);

		//if (devices.size() == 1)
		//{
		//	physicalDevice = devices[0];
		//}
		//else if (devices.size() > 1)
		//{
		//	// sort by scoring the supported properties and features
		//	auto DeviceScore = [this](VkPhysicalDevice device)
		//	{
		//		if (!IsDeviceSuitable(device))
		//		{
		//			return -1;
		//		}

		//		int score = 0;
		//		VkPhysicalDeviceProperties properties;
		//		VkPhysicalDeviceFeatures features;
		//		vkGetPhysicalDeviceProperties(device, &properties);
		//		vkGetPhysicalDeviceFeatures(device, &features);

		//		// discrete gpu has performance advantage
		//		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		//		{
		//			score += 1000;
		//		}

		//		// maximum possible size of textures affects graphics quality
		//		score += properties.limits.maxImageDimension2D;

		//		/* uncomment this if geometry shader support is required
		//		if (!features.geometryShader)
		//		{
		//			score = 0;
		//		}
		//		*/

		//		return score;
		//	};

		//	std::sort(devices.begin(), devices.end(),
		//		[&](auto A, auto B)
		//		{
		//			return DeviceScore(A) > DeviceScore(B);
		//		});

		//	physicalDevice = devices[0];
		//}

		//Check(physicalDevice != VK_NULL_HANDLE, "Failed to find suitable GPU!");


		//VkPhysicalDeviceProperties properties;
		//vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		//LogInfo("Device Name: %s", properties.deviceName);

		//msaaSamples = GetMaxUsableSampleCount(physicalDevice);
	}

	std::vector<VkPhysicalDevice> VulkanPhysicalDevice::GetAllPhysicalDevices(VkInstance vulkanInstance)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);

		Check(deviceCount > 0, "No vulkan-supported GPU is found!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

		return devices;
	}

	bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		//QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device, surface);

		//bool bExtensionSupported = CheckDeviceExtensionSupport();

		//bool bSwapChainAdequate = false;
		//if (bExtensionSupported)
		//{
		//	SwapChainSupportDetails supportDetails = QuerySwapChainSupport(device, surface);
		//	bSwapChainAdequate = !supportDetails.surfaceFormats.empty() && !supportDetails.presentModes.empty();
		//}

		//VkPhysicalDeviceFeatures supportedFeatures;
		//vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

		//return queueFamilyIndices.IsComplete()
		//	&& bExtensionSupported
		//	&& bSwapChainAdequate
		//	&& supportedFeatures.samplerAnisotropy;

		return false;
	}
}