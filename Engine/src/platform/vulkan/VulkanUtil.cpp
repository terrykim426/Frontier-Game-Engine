#include "pch.h"
#include "platform/vulkan/VulkanUtil.h"
#include "platform/vulkan/VulkanLogicalDevice.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FGEngine
{
	VkImageView VulkanUtil::CreateImageView(VulkanLogicalDevice* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VkResult result = vkCreateImageView(*device, &createInfo, nullptr, &imageView);
		Check(result == VK_SUCCESS, "Failed to create image view. Vulkan error: %d", result);

		return imageView;
	}
}
