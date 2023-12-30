#pragma once

#include "vulkan/vulkan_core.h"

namespace FGEngine
{
	class VulkanLogicalDevice;

	class VulkanUtil
	{
	public:
		static VkImageView CreateImageView(const std::shared_ptr<VulkanLogicalDevice>& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);


		template <typename T>
		static void VectorDestroy(void(*DestroyFunc)(VkDevice, T, const VkAllocationCallbacks*), VkDevice device, std::vector<T>& dataVector, const VkAllocationCallbacks* callback)
		{
			for (const T& data : dataVector)
			{
				DestroyFunc(device, data, callback);
			}
			dataVector.clear();
		}
	};
}

