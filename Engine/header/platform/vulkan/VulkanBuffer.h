#pragma once

#include "vulkan/vulkan_core.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "platform/vulkan/VulkanCommand.h"

namespace FGEngine
{
	enum class EBufferType : int8_t
	{
		Vertex,
		Index,
	};

	class VulkanBuffer
	{
	public:
		VulkanBuffer(const std::shared_ptr<VulkanLogicalDevice>& inLogicalDevice);

		~VulkanBuffer();

		template<typename T>
		void Init(
			const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice,
			const std::shared_ptr<VulkanCommand>& command,
			std::vector<T> inBuffer,
			EBufferType bufferType)
		{
			VkDeviceSize bufferSize = sizeof(inBuffer[0]) * inBuffer.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			CreateBuffer(physicalDevice, logicalDevice,
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
			{
				memcpy(data, inBuffer.data(), (size_t)bufferSize);
			}
			vkUnmapMemory(*logicalDevice, stagingBufferMemory);

			VkBufferUsageFlagBits usageFlagBit = (VkBufferUsageFlagBits)0;
			switch (bufferType)
			{
			case FGEngine::EBufferType::Vertex:
				usageFlagBit = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				break;
			case FGEngine::EBufferType::Index:
				usageFlagBit = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				break;
			}
			Check(usageFlagBit, "Usage flag not set.");

			CreateBuffer(physicalDevice, logicalDevice, 
				bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlagBit,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				buffer, bufferMemory);

			command->CopyBuffer(stagingBuffer, buffer, bufferSize);

			vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
			vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
		}

		operator VkBuffer() const { return buffer; }

	public:
		static void CreateBuffer(
			const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, 
			const std::shared_ptr<VulkanLogicalDevice>& logicalDevice,
			VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	private:
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;

		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};
}

