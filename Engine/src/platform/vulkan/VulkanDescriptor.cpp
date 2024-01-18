#include "pch.h"
#include "platform/vulkan/VulkanDescriptor.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "platform/vulkan/VulkanTextureImageView.h"

#include <array>

namespace FGEngine
{
	VulkanDescriptor::VulkanDescriptor(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, uint32_t descriptorCount)
	{
		logicalDevice = inLogicalDevice;

		CreateDescriptorSetLayout();
		CreateDescriptorPool(descriptorCount);
	}

	VulkanDescriptor::~VulkanDescriptor()
	{
		vkDestroyDescriptorPool(*logicalDevice, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(*logicalDevice, descriptorSetLayout, nullptr);
	}

	void VulkanDescriptor::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutCreateInfo.pBindings = bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(*logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
		Check(result == VK_SUCCESS, "Failed to create descriptor set layout. Vulkan error: %d", result);
	}

	void VulkanDescriptor::CreateDescriptorPool(uint32_t descriptorCount)
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = descriptorCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = descriptorCount;

		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.maxSets = descriptorCount;

		VkResult result = vkCreateDescriptorPool(*logicalDevice, &createInfo, nullptr, &descriptorPool);
		Check(result == VK_SUCCESS, "Failed to create descriptor pool. Vulkan error: %d", result);
	}

	void VulkanDescriptor::CreateDescriptorSets(uint32_t descriptorCount, std::vector<VkBuffer> uniformBuffers, uint64_t uniformBufferObjectSize, std::shared_ptr<VulkanTextureImageView> textureImageView)
	{
		std::vector<VkDescriptorSetLayout> layouts(descriptorCount, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = descriptorCount;
		allocateInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(descriptorCount);
		VkResult result = vkAllocateDescriptorSets(*logicalDevice, &allocateInfo, descriptorSets.data());
		Check(result == VK_SUCCESS, "Failed to allocate descriptor sets. Vulkan error: %d", result);

		for (size_t i = 0; i < descriptorCount; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = uniformBufferObjectSize;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = *textureImageView;
			imageInfo.sampler = textureImageView->GetSampler();

			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = descriptorSets[i];
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].dstArrayElement = 0;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[0].pBufferInfo = &bufferInfo;

			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].dstSet = descriptorSets[i];
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].dstArrayElement = 0;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(*logicalDevice,
				static_cast<uint32_t>(writeDescriptorSets.size()),
				writeDescriptorSets.data(),
				0, nullptr);
		}
	}
}