#pragma once

#include "vulkan/vulkan_core.h"

#include <memory>

namespace FGEngine
{
	class VulkanLogicalDevice;
	class Shader;

	class VulkanShaderModule
	{
	public:
		VulkanShaderModule(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, const Shader& shader);
		~VulkanShaderModule();

		operator VkShaderModule () const { return shaderModule; }

		VkPipelineShaderStageCreateInfo GetCreateInfo() const { return shaderStageCreateInfo; }

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkShaderModule shaderModule;
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
	};
}

