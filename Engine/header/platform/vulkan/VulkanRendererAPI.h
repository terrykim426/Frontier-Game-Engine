#pragma once

#include "renderer/RendererAPI.h"
#include "renderer/Vertex.h"

#include <vector>
#include <optional>
#include <memory>


#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace FGEngine
{
	class VulkanInstance;
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapChain;
	class VulkanPipeline;
	class VulkanCommand;
	class VulkanImageView;
	class VulkanTextureImageView;
	class VulkanBuffer;

	class Texture;

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
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateColorResources();
		void CreateDepthResources();
		void LoadModel();
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateSyncObjects();

		void RecreateSwapChain();

		void UpdateUniformBuffer(uint32_t currentImage);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	private:
		GLFWwindow* nativeWindow;

		std::shared_ptr<VulkanInstance> vulkanInstance;
		std::shared_ptr<VulkanPhysicalDevice> physicalDevice;
		std::shared_ptr<VulkanLogicalDevice> logicalDevice;
		std::shared_ptr<VulkanSwapChain> swapChain;

		VkRenderPass renderPass;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		std::shared_ptr<VulkanPipeline> graphicsPipeline;
		std::shared_ptr<VulkanCommand> command;

		std::shared_ptr<VulkanBuffer> vertexBuffer;
		std::shared_ptr<VulkanBuffer> indexBuffer;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;


		VkClearColorValue clearColor;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		std::shared_ptr<VulkanTextureImageView> textureImageView;

		std::shared_ptr<VulkanImageView> colorImageView;
		std::shared_ptr<VulkanImageView> depthImageView;

		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		uint32_t currentFrame = 0;
		bool bResizeRequested = false;

		std::vector<const char*> deviceExtensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const int MAX_FRAMES_IN_FLIGHT = 2;

		// TODO: To move to it own class
#pragma region Vertex Buffer
	private:

		class Model* model = nullptr;
#pragma endregion

#pragma region Descriptor
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 projection;
		};
#pragma endregion

	};
}

