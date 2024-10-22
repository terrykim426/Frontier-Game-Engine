#pragma once

#include "vulkan/vulkan_core.h"

#include <memory>

namespace FGEngine
{
	class VulkanLogicalDevice;
	class VulkanTextureImageView;

	class VulkanDescriptor
	{
	public:
		VulkanDescriptor(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, uint32_t descriptorCount);
		~VulkanDescriptor();

	private:
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool(uint32_t descriptorCount);

	public:
		void CreateDescriptorSets(uint32_t descriptorCount, std::vector<VkBuffer> uniformBuffers, uint64_t uniformBufferObjectSize, std::shared_ptr<VulkanTextureImageView> textureImageView);

		VkDescriptorSetLayout GetSetLayout() const { return descriptorSetLayout; }
		VkDescriptorSet GetSet(int32_t Index) const 
		{
			if (Index < 0 || Index >= descriptorSets.size()) return nullptr;
			return descriptorSets[Index]; 
		}

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
	};
}

