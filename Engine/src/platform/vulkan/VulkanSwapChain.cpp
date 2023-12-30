#include "pch.h"
#include "platform/vulkan/VulkanSwapChain.h"
#include "platform/vulkan/VulkanInstance.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "platform/vulkan/VulkanUtil.h"

#include <algorithm>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FGEngine
{
#pragma region Helpers
	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
	{
		Check(availableSurfaceFormats.size() > 0, "No surface format available!");

		for (const VkSurfaceFormatKHR& surfaceFormat : availableSurfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return surfaceFormat;
			}
		}

		// can't find suitable one, just return the first one
		return availableSurfaceFormats[0];
	}

	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		Check(availablePresentModes.size() > 0, "No present mode available!");

		for (const VkPresentModeKHR& presentMode : availablePresentModes)
		{
			// mailbox uses more energy but less latency as it try to use most up-to-date buffer 
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return presentMode;
			}
		}

		// similar to vsync
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t windowWidth, uint32_t windowHeight)
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) // NOTE: the "max" is conflicting with the max marco, hence its used this way
		{
			return capabilities.currentExtent;
		}

		VkExtent2D actualExtent{};
		actualExtent.width = std::clamp(windowWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(windowHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
#pragma endregion

	VulkanSwapChain::VulkanSwapChain(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, GLFWwindow* nativeWindow)
	{
		logicalDevice = inLogicalDevice;

		CreateSwapChain(vulkanInstance, physicalDevice, nativeWindow);
		CreateImageViews();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		CleanUp();
	}

	void VulkanSwapChain::Recreate(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, GLFWwindow* nativeWindow)
	{
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(nativeWindow, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(*logicalDevice);

		CleanUp();

		CreateSwapChain(vulkanInstance, physicalDevice, nativeWindow);
		CreateImageViews();
		//CreateColorResources();
		//CreateDepthResources();
		//CreateFrameBuffers();
	}

	void VulkanSwapChain::CreateSwapChain(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, GLFWwindow* nativeWindow)
	{
		SwapChainSupportDetails supportDetails = physicalDevice->GetSwapChainSupportDetails();
		int width, height;
		glfwGetFramebufferSize(nativeWindow, &width, &height);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(supportDetails.surfaceFormats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(supportDetails.presentModes);
		extent = ChooseSwapExtent(supportDetails.capabilities, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		unsigned imageCount = [&] {
			/* if maxImageCount is zero, it means there's no maximum.
			*  if that's the case just use the min, with 1 more additional,
			*  to reduce the chance of waiting for the driver to complete its instructions
			*
			*  ref: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#:~:text=by%20the%20implementation.-,Creating%20the%20swap%20chain,-Now%20that%20we
			*/
			if (supportDetails.capabilities.maxImageCount == 0)
			{
				return supportDetails.capabilities.minImageCount + 1;
			}
			else
			{
				return supportDetails.capabilities.maxImageCount;
			}
		}();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = vulkanInstance->GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // this is always 1, unless it is developing for stereoscopic 3D application
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // render directly. If want to render to a separate image, use VK_IMAGE_USAGE_TRANSFER_DST_BIT instead.


		QueueFamilyIndices indices = physicalDevice->GetQueueFamilyIndices();
		if (indices.graphicsFamily != indices.presentFamily)
		{
			uint32_t queueFamilyIndices[]
			{
				indices.graphicsFamily.value(),
				indices.presentFamily.value()
			};

			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = supportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(*logicalDevice, &createInfo, nullptr, &swapChain);
		Check(result == VK_SUCCESS, "Failed to create swap chain. Vulkan error: %d", result);

		uint32_t swapChainImageCount;
		vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &swapChainImageCount, nullptr);
		if (swapChainImageCount > 0)
		{
			images.resize(swapChainImageCount);
			vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &swapChainImageCount, images.data());
		}

		imageFormat = surfaceFormat.format;
	}

	void VulkanSwapChain::CreateImageViews()
	{
		imageViews.resize(images.size());

		for (size_t i = 0; i < images.size(); i++)
		{
			imageViews[i] = VulkanUtil::CreateImageView(logicalDevice, images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void VulkanSwapChain::CleanUp()
	{
		//vkDestroyImageView(*logicalDevice, colorImageView, nullptr);
		//vkDestroyImage(*logicalDevice, colorImage, nullptr);
		//vkFreeMemory(*logicalDevice, colorImageMemory, nullptr);

		//vkDestroyImageView(*logicalDevice, depthImageView, nullptr);
		//vkDestroyImage(*logicalDevice, depthImage, nullptr);
		//vkFreeMemory(*logicalDevice, depthImageMemory, nullptr);

		VulkanUtil::VectorDestroy(vkDestroyFramebuffer, *logicalDevice, frameBuffers, nullptr);
		VulkanUtil::VectorDestroy(vkDestroyImageView, *logicalDevice, imageViews, nullptr);
		vkDestroySwapchainKHR(*logicalDevice, swapChain, nullptr);
	}

	void VulkanSwapChain::CreateFrameBuffers(VkImageView colorImageView, VkImageView depthImageView, VkRenderPass renderPass)
	{
		frameBuffers.resize(imageViews.size());

		for (size_t i = 0; i < imageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				imageViews[i]
			};

			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.renderPass = renderPass;
			createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createInfo.pAttachments = attachments.data();
			createInfo.width = extent.width;
			createInfo.height = extent.height;
			createInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(*logicalDevice, &createInfo, nullptr, &frameBuffers[i]);
			Check(result == VK_SUCCESS, "Failed to create frame buffer %d. Vulkan error: %d", i, result);
		}
	}
}
