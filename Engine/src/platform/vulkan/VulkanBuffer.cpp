#include "pch.h"
#include "platform/vulkan/VulkanBuffer.h"

namespace FGEngine
{
	VulkanBuffer::VulkanBuffer(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice)
	{
		logicalDevice = inLogicalDevice;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		vkDestroyBuffer(*logicalDevice, buffer, nullptr);
		vkFreeMemory(*logicalDevice, bufferMemory, nullptr);
	}

	void VulkanBuffer::CreateBuffer(
		const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice,
		const std::shared_ptr<VulkanLogicalDevice>& logicalDevice,
		VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.flags = 0;

		VkResult result = vkCreateBuffer(*logicalDevice, &bufferCreateInfo, nullptr, &buffer);
		Check(result == VK_SUCCESS, "Failed to create vertex buffer. Vulkan error: %d", result);

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(*logicalDevice, buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = physicalDevice->FindMemoryType(memoryRequirements.memoryTypeBits, properties);

		// NOTE: shouldn't do allocation for each individual buffer, instead should create a custom allocator
		// ref: https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer#:~:text=more%20complex%20geometry.-,Conclusion,-It%20should%20be
		result = vkAllocateMemory(*logicalDevice, &allocateInfo, nullptr, &bufferMemory);
		Check(result == VK_SUCCESS, "Failed to allocate vertex buffer memory. Vulkan error: %d", result);

		vkBindBufferMemory(*logicalDevice, buffer, bufferMemory, 0);
	}
}
