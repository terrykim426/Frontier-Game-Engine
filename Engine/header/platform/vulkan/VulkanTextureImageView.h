#pragma once

#include "vulkan/vulkan_core.h"

#include <memory>

namespace FGEngine
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapChain;
	class VulkanCommand;

	class Texture;

	class VulkanTextureImageView
	{
	public:
		VulkanTextureImageView(
			const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice,
			const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice,
			const std::shared_ptr<VulkanSwapChain>& swapChain,
			const std::shared_ptr<VulkanCommand>& command,
			const Texture& texture);

		~VulkanTextureImageView();

		operator VkImageView() const { return imageView; }

		VkSampler GetSampler() const { return sampler; }

	private:
		void CreateTextureSampler(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice);

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		uint32_t mipLevels;
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;
	};
}

