#include "pch.h"
#include "platform/vulkan/VulkanInstance.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace FGEngine
{
	VulkanInstance::VulkanInstance(GLFWwindow* nativeWindow, const VulkanInstanceParameters& parameters)
	{
		CreateInstance(parameters);
		CreateSurface(nativeWindow);
	}

	VulkanInstance::~VulkanInstance()
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	bool VulkanInstance::IsValidationLayerSupported() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// check that all validation layers are available
		for (const char* layer : validationLayers)
		{
			bool bLayerFound = false;
			for (const VkLayerProperties& property : availableLayers)
			{
				if (strcmp(layer, property.layerName))
				{
					bLayerFound = true;
					break;
				}
			}

			if (!bLayerFound)
			{
				return false;
			}
		}

		return true;
	}

	void VulkanInstance::CreateInstance(const VulkanInstanceParameters& parameters)
	{
		if (bEnableValidationLayers)
		{
			Check(IsValidationLayerSupported(), "Validation requested but not available");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = parameters.applicationName.c_str();
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);	// TODO: should include this in the setting?
		appInfo.pEngineName = parameters.engineName.c_str();
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);		// TODO: should include this in the setting?
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		createInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		createInfo.enabledExtensionCount = glfwExtensionCount;

		uint32_t layerCount = 0;
		if (bEnableValidationLayers)
		{
			layerCount += static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		createInfo.enabledLayerCount = layerCount;

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS)
		{
			LogAssert("Failed to create vulkan instance. Vulkan error: %d", result);

			/* Note: there might be a case where it doesn't work on mac.
			* https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance#:~:text=is%20created%20successfully.-,Encountered%20VK_ERROR_INCOMPATIBLE_DRIVER,-%3A
			*/
		}
	}

	void VulkanInstance::CreateSurface(GLFWwindow* nativeWindow)
	{
		Check(nativeWindow, "No window supplied!");

		VkResult result = glfwCreateWindowSurface(instance, nativeWindow, nullptr, &surface);
		Check(result == VK_SUCCESS, "Failed to create surface! Vulkan error: %d", result);
	}
}
