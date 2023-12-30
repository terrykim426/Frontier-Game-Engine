#pragma once

#include "vulkan/vulkan_core.h"

#include <memory>

struct GLFWwindow;

namespace FGEngine
{
	class VulkanInstance;
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, GLFWwindow* nativeWindow);
		~VulkanSwapChain();

		void Recreate(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, GLFWwindow* nativeWindow);

		operator VkSwapchainKHR () const { return swapChain; }

	private:
		void CreateSwapChain(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, GLFWwindow* nativeWindow);
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
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> frameBuffers;
		VkFormat imageFormat;
		VkExtent2D extent;
	};
}

