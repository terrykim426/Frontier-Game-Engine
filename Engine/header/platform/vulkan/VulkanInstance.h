#pragma once

#include <string>

struct GLFWwindow;
struct VkInstance_T;
struct VkSurfaceKHR_T;

namespace FGEngine
{
	struct VulkanInstanceParameters
	{
		std::string applicationName;
		std::string engineName;
	};

	class VulkanInstance
	{
	public:
		VulkanInstance(GLFWwindow* nativeWindow, const VulkanInstanceParameters& parameters);
		~VulkanInstance();

	private:
		bool IsValidationLayerSupported() const;
		void CreateInstance(const VulkanInstanceParameters& parameters);
		void CreateSurface(GLFWwindow* nativeWindow);

	public:
		operator VkInstance_T*() const { return instance; }
		VkSurfaceKHR_T* GetSurface() const { return surface; }

	public:
#ifdef NDEBUG
		const bool bEnableValidationLayers = false;
#else
		const bool bEnableValidationLayers = true;
#endif
		const std::vector<const char*> validationLayers
		{
			"VK_LAYER_KHRONOS_validation"
		};

	private:
		VkInstance_T* instance;
		VkSurfaceKHR_T* surface;
	};
}
