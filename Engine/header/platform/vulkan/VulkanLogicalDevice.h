#pragma once

struct VkDevice_T;
struct VkQueue_T;

namespace FGEngine
{
	class VulkanInstance;
	class VulkanPhysicalDevice;

	class VulkanLogicalDevice
	{
	public:
		VulkanLogicalDevice(const std::shared_ptr<VulkanInstance>& vulkanInstance, const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::vector<const char*>& deviceExtensions);
		~VulkanLogicalDevice();

		operator VkDevice_T* () const { return device; }

		VkQueue_T* GetGraphicsQueue() const { return graphicsQueue; }
		VkQueue_T* GetPresentQueue() const { return presentQueue; }

	private:
		VkDevice_T* device;
		VkQueue_T* graphicsQueue;
		VkQueue_T* presentQueue;
	};
}

