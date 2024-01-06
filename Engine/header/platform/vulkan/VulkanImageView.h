#pragma once

#include "vulkan/vulkan_core.h"

namespace FGEngine
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapChain;

	struct VulkanImageViewSetting
	{
		VkSampleCountFlagBits msaaSamples;
		VkFormat imageFormat;
		VkImageUsageFlags imageUsageFlags;
		VkImageAspectFlags aspectFlags;
	};

	class VulkanImageView
	{
	public:
		VulkanImageView(
			const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice,
			const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice,
			const std::shared_ptr<VulkanSwapChain>& swapChain, 
			const VulkanImageViewSetting& imageViewSetting);

		~VulkanImageView();

		operator VkImageView () const { return imageView; }

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	};
}

