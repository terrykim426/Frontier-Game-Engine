#pragma once

#include "vulkan/vulkan_core.h"

struct GLFWwindow;

namespace FGEngine
{
	class VulkanInstance;
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(VulkanInstance* vulkanInstance, VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* inLogicalDevice, GLFWwindow* nativeWindow);
		~VulkanSwapChain();

		void Recreate(VulkanInstance* vulkanInstance, VulkanPhysicalDevice* physicalDevice, GLFWwindow* nativeWindow);

		operator VkSwapchainKHR () const { return swapChain; }

	private:
		void CreateSwapChain(VulkanInstance* vulkanInstance, VulkanPhysicalDevice* physicalDevice, GLFWwindow* nativeWindow);
		void CreateImageViews();
		void CleanUp();

	public:
		void CreateFrameBuffers(VkImageView colorImageView, VkImageView depthImageView, VkRenderPass renderPass);

	public:
		VkFormat GetImageFormat() const { return imageFormat; }
		VkExtent2D GetExtent() const { return extent; }
		VkFramebuffer GetFrameBuffer(uint32_t index) const
		{
			if (index < 0 || index >= frameBuffers.size()) return nullptr;
			return frameBuffers[index];
		}

	private:
		VulkanLogicalDevice* logicalDevice;	// TODO: change it to share ptr?

		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> frameBuffers;
		VkFormat imageFormat;
		VkExtent2D extent;
	};
}

