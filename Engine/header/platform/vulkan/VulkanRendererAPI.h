#pragma once

#include "renderer/RendererAPI.h"

#include <vector>
#include <optional>

#pragma region VertexBuffer
#include <array>
#pragma endregion

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

namespace FGEngine
{
	class VulkanRendererAPI : public IRendererAPI
	{
	public:
		VulkanRendererAPI(const RendererProperties& rendererProperties);
		virtual ~VulkanRendererAPI() override;

		// Inherited via IRendererAPI
		virtual void SetClearColor(float r, float g, float b, float a) override;
		virtual void Clear() const override;
		virtual void Render(void* nativeWindow) override;
		virtual void Resize() override;

		virtual std::string GetName() const override;
		virtual std::string GetVersion() const override;

	public:
		static bool IsSupported();

	private:
		void CreateInstance();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		void CleanUpSwapChain();
		void RecreateSwapChain();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		bool CheckValidationLayerSupport();
		bool CheckDeviceExtensionSupport();
		bool IsDeviceSuitable(VkPhysicalDevice device);

	private:
		GLFWwindow* nativeWindow;

		VkInstance vulkanInstance;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice = VK_NULL_HANDLE;
		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFrameBuffers;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		VkClearValue clearColor;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		uint32_t currentFrame = 0;
		bool bResizeRequested = false;

		std::vector<const char*> validationLayers
		{
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> deviceExtensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		const int MAX_FRAMES_IN_FLIGHT = 2;

		// TODO: To move to it own class
#pragma region Vertex Buffer
	private:
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;

			static VkVertexInputBindingDescription GetBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				return attributeDescriptions;
			}
		};

		const std::vector<Vertex> vertices =
		{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};
#pragma endregion
	};
}
