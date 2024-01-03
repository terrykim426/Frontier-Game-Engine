#include "pch.h"
#include "platform/vulkan/VulkanShaderModule.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "renderer/Shader.h"

namespace FGEngine
{
	VulkanShaderModule::VulkanShaderModule(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice, const Shader& shader)
	{
		logicalDevice = inLogicalDevice;

		std::vector<char> shaderCode = shader.GetShaderCode();
		Check(shaderCode.size() > 0, "Shader code from (%s) is empty!", shader.GetShaderFilename());

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkResult result = vkCreateShaderModule(*logicalDevice, &createInfo, nullptr, &shaderModule);

		// shader stage create info
		{
			shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageCreateInfo.module = shaderModule;
			shaderStageCreateInfo.pName = "main";
			switch (shader.GetType())
			{
			case Shader::EType::Vertex:
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case Shader::EType::Fragment:
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			}
		}
	}

	VulkanShaderModule::~VulkanShaderModule()
	{
		vkDestroyShaderModule(*logicalDevice, shaderModule, nullptr);
	}
}