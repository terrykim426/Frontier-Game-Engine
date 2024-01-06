#pragma once

#include "vulkan/vulkan_core.h"
#include "renderer/Vertex.h"

#include <array>

namespace FGEngine
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;

	class VulkanUtil
	{
	public:
		static void CreateImage(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, 
			uint32_t width, uint32_t height, uint32_t mipLevels,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		static VkImageView CreateImageView(const std::shared_ptr<VulkanLogicalDevice>& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);


		template <typename T>
		static void VectorDestroy(void(*DestroyFunc)(VkDevice, T, const VkAllocationCallbacks*), VkDevice device, std::vector<T>& dataVector, const VkAllocationCallbacks* callback)
		{
			for (const T& data : dataVector)
			{
				DestroyFunc(device, data, callback);
			}
			dataVector.clear();
		}
	};

	struct VertexHelper
	{
		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};
}

