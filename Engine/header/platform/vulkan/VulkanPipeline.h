#pragma once

#include "vulkan/vulkan_core.h"

#include <memory>

namespace FGEngine
{
	class VulkanLogicalDevice;
	class VulkanShaderModule;

	struct VulkanPipelineSetting
	{
		std::shared_ptr<VulkanShaderModule> vertexShaderModule;
		std::shared_ptr<VulkanShaderModule> fragmentShaderModule;

		VkSampleCountFlagBits msaaSamples;
		VkDescriptorSetLayout descriptorSetLayout;
		VkRenderPass renderPass;
	};

	class VulkanPipeline
	{
	public:
		VulkanPipeline(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, const VulkanPipelineSetting& setting);
		~VulkanPipeline();

		operator VkPipeline () const { return graphicsPipeline; }

		VkPipelineLayout GetLayout() const { return pipelineLayout; }

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
	};
}

