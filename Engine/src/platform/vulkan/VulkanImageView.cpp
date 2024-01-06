#include "pch.h"
#include "platform/vulkan/VulkanImageView.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "platform/vulkan/VulkanSwapChain.h"
#include "platform/vulkan/VulkanUtil.h"

namespace FGEngine
{
	VulkanImageView::VulkanImageView(
		const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice,
		const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, 
		const std::shared_ptr<VulkanSwapChain>& swapChain, 
		const VulkanImageViewSetting& imageViewSetting)
	{
		logicalDevice = inLogicalDevice;

		VulkanUtil::CreateImage(physicalDevice, logicalDevice,
			swapChain->GetExtent().width, swapChain->GetExtent().height, 1,
			imageViewSetting.msaaSamples, imageViewSetting.imageFormat, VK_IMAGE_TILING_OPTIMAL,
			imageViewSetting.imageUsageFlags,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			image, imageMemory);

		imageView = VulkanUtil::CreateImageView(logicalDevice, image, imageViewSetting.imageFormat, imageViewSetting.aspectFlags, 1);
	}

	VulkanImageView::~VulkanImageView()
	{
		vkDestroyImageView(*logicalDevice, imageView, nullptr);
		vkDestroyImage(*logicalDevice, image, nullptr);
		vkFreeMemory(*logicalDevice, imageMemory, nullptr);
	}
}
