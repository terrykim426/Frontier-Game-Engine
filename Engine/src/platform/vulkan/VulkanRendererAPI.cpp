#include "pch.h"
#include "platform/vulkan/VulkanRendererAPI.h"
#include "platform/vulkan/VulkanDefineType.h"
#include "platform/vulkan/VulkanInstance.h"
#include "platform/vulkan/VulkanPhysicalDevice.h"
#include "platform/vulkan/VulkanLogicalDevice.h"
#include "platform/vulkan/VulkanSwapChain.h"
#include "platform/vulkan/VulkanPipeline.h"
#include "platform/vulkan/VulkanCommand.h"
#include "platform/vulkan/VulkanShaderModule.h"

#include "core/Logger.h"
#include "renderer/Texture.h"
#include "renderer/Model.h"
#include "renderer/Shader.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <set>
#include <array>

#include <filesystem>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>


namespace FGEngine
{
#pragma	region	Utility
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);	// read from the end, to determine the file size

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0); // return to beginning
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	template <typename T>
	static void VectorDestroy(void(*DestroyFunc)(VkDevice, T, const VkAllocationCallbacks*), VkDevice device, std::vector<T>& dataVector, const VkAllocationCallbacks* callback)
	{
		for (const T& data : dataVector)
		{
			DestroyFunc(device, data, callback);
		}
		dataVector.clear();
	}

	static uint32_t FindMemoryType(VkPhysicalDevice phyiscalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(phyiscalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if (!(typeFilter & (1 << i))) continue;

			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		NoEntry("Failed to find suitable memory type");
	}

	static VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		NoEntry("Failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}

	static VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
	{
		return FindSupportedFormat(physicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	static bool HasStencilComponent(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return true;
		}

		return false;
	}

	static VkCommandBuffer BeginSingleTimeCommands(const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, VkCommandPool commandPool)
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = commandPool;
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(*logicalDevice, &allocateInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	static void EndSingleTimeCommands(const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(logicalDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(logicalDevice->GetGraphicsQueue());

		vkFreeCommandBuffers(*logicalDevice, commandPool, 1, &commandBuffer);
	}

	static void CopyBuffer(const std::shared_ptr<VulkanLogicalDevice> logicalDevice, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);
		{
			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;

			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		}
		EndSingleTimeCommands(logicalDevice, commandPool, commandBuffer);
	}

	static void TransitionImageLayout(const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else
			{
				NoEntry("Unsupported layout transition!");
			}


			vkCmdPipelineBarrier(commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		}
		EndSingleTimeCommands(logicalDevice, commandPool, commandBuffer);
	}

	static void CopyBufferToImage(const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);
		{
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0,0,0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(commandBuffer, buffer, image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}
		EndSingleTimeCommands(logicalDevice, commandPool, commandBuffer);
	}

	VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VkResult result = vkCreateImageView(device, &createInfo, nullptr, &imageView);
		Check(result == VK_SUCCESS, "Failed to create image view. Vulkan error: %d", result);

		return imageView;
	}

	void GenerateMipmaps(VkPhysicalDevice physicalDevice, const std::shared_ptr<VulkanLogicalDevice>& logicalDevice, VkCommandPool commandPool, VkImage image, VkFormat imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevels)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

		Check(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "Texture image format does not support linear blitting!");

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = textureWidth;
		int32_t mipHeight = textureHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0,0,0 };
			blit.srcOffsets[1] = { mipWidth,mipHeight,1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0,0,0 };
			blit.dstOffsets[1] =
			{
				mipWidth > 1 ? mipWidth / 2 : 1,
				mipHeight > 1 ? mipHeight / 2 : 1,
				1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(logicalDevice, commandPool, commandBuffer);
	}
#pragma endregion

#pragma region VulkanRendererAPI
	VulkanRendererAPI::VulkanRendererAPI(const RendererProperties& rendererProperties)
	{
		nativeWindow = rendererProperties.nativeWindow;
		Check(nativeWindow, "No window supplied!");

		vulkanInstance = std::make_shared<VulkanInstance>(nativeWindow,
			VulkanInstanceParameters
			{
			"Temp Frontier Application Name",	// TODO: to be forwarded from application layer
			"Frontier Game Engine"				// TODO: hardcode this at the header file?
			});

		physicalDevice = std::make_shared<VulkanPhysicalDevice>(vulkanInstance, deviceExtensions);
		msaaSamples = physicalDevice->GetMaxSampleCount();

		logicalDevice = std::make_shared<VulkanLogicalDevice>(vulkanInstance, physicalDevice, deviceExtensions);

		swapChain = std::make_shared<VulkanSwapChain>(vulkanInstance, physicalDevice, logicalDevice, nativeWindow);

		CreateRenderPass();
		CreateDescriptorSetLayout();

		Shader vertShader("shader/TestShaderVert.spv", Shader::EType::Vertex);
		Shader fragShader("shader/TestShaderFrag.spv", Shader::EType::Fragment);

		VulkanPipelineSetting pipelineSetting;
		pipelineSetting.vertexShaderModule = std::make_shared<VulkanShaderModule>(logicalDevice, vertShader);
		pipelineSetting.fragmentShaderModule = std::make_shared<VulkanShaderModule>(logicalDevice, fragShader);
		pipelineSetting.msaaSamples = msaaSamples;
		pipelineSetting.descriptorSetLayout = descriptorSetLayout;
		pipelineSetting.renderPass = renderPass;

		graphicsPipeline = std::make_shared<VulkanPipeline>(logicalDevice, pipelineSetting);

		CreateColorResources();
		CreateDepthResources();

		swapChain->CreateFrameBuffers(colorImageView, depthImageView, renderPass);

		command = std::make_shared<VulkanCommand>(physicalDevice, logicalDevice, MAX_FRAMES_IN_FLIGHT);

		LoadModel();

		CreateTextureImage(*model->GetTexture());
		CreateTextureImageView();
		CreateTextureSampler();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffer();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateSyncObjects();
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
		vkDeviceWaitIdle(*logicalDevice);

		VectorDestroy(vkDestroySemaphore, *logicalDevice, imageAvailableSemaphores, nullptr);
		VectorDestroy(vkDestroySemaphore, *logicalDevice, renderFinishedSemaphores, nullptr);
		VectorDestroy(vkDestroyFence, *logicalDevice, inFlightFences, nullptr);

		vkDestroyBuffer(*logicalDevice, indexBuffer, nullptr);
		vkFreeMemory(*logicalDevice, indexBufferMemory, nullptr);
		vkDestroyBuffer(*logicalDevice, vertexBuffer, nullptr);
		vkFreeMemory(*logicalDevice, vertexBufferMemory, nullptr);

		if (model)
		{
			delete model;
			model = nullptr;
		}

		CleanUpSwapChain();

		vkDestroySampler(*logicalDevice, textureSampler, nullptr);
		vkDestroyImageView(*logicalDevice, textureImageView, nullptr);
		vkDestroyImage(*logicalDevice, textureImage, nullptr);
		vkFreeMemory(*logicalDevice, textureImageMemory, nullptr);

		VectorDestroy(vkDestroyBuffer, *logicalDevice, uniformBuffers, nullptr);
		VectorDestroy(vkFreeMemory, *logicalDevice, uniformBuffersMemory, nullptr);

		vkDestroyDescriptorPool(*logicalDevice, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(*logicalDevice, descriptorSetLayout, nullptr);

		vkDestroyRenderPass(*logicalDevice, renderPass, nullptr);
	}

	void VulkanRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		clearColor = { r,g,b,a };
	}

	void VulkanRendererAPI::Clear() const
	{
	}

	void VulkanRendererAPI::Render(void* nativeWindow)
	{
		vkWaitForFences(*logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(*logicalDevice, *swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		Check(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image. Vulkan error: %d", result);

		vkResetFences(*logicalDevice, 1, &inFlightFences[currentFrame]);

		VkCommandBuffer commandBuffer;
		command->GetBuffer(currentFrame, commandBuffer);
		vkResetCommandBuffer(commandBuffer, 0);
		RecordCommandBuffer(commandBuffer, imageIndex);

		UpdateUniformBuffer(currentFrame);

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		result = vkQueueSubmit(logicalDevice->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);
		Check(result == VK_SUCCESS, "Failed to submit draw command buffer. Vulkan error: %d", result);

		VkSwapchainKHR swapChains[] = { *swapChain };
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(logicalDevice->GetPresentQueue(), &presentInfo);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || bResizeRequested)
		{
			bResizeRequested = false;
			RecreateSwapChain();
			return;
		}
		Check(result == VK_SUCCESS, "Failed to present swap chain image. Vulkan error: %d", result);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRendererAPI::Resize()
	{
		bResizeRequested = true;
	}

	std::string VulkanRendererAPI::GetName() const
	{
		return "Vulkan";
	}

	std::string VulkanRendererAPI::GetVersion() const
	{
		uint32_t instanceVersion = VK_API_VERSION_1_0;
		auto FN_vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
		if (FN_vkEnumerateInstanceVersion)
		{
			vkEnumerateInstanceVersion(&instanceVersion);
		}

		// 3 macros to extract version info
		uint32_t major = VK_API_VERSION_MAJOR(instanceVersion);
		uint32_t minor = VK_API_VERSION_MINOR(instanceVersion);
		uint32_t patch = VK_API_VERSION_PATCH(instanceVersion);

		char result[50];
		sprintf_s(result, "%d.%d.%d", major, minor, patch);
		return result;
	}

	bool VulkanRendererAPI::IsSupported()
	{
		return glfwVulkanSupported();
	}

	void VulkanRendererAPI::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachmentDescription{};
		colorAttachmentDescription.format = swapChain->GetImageFormat();
		colorAttachmentDescription.samples = msaaSamples;
		colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachmentDescription{};
		depthAttachmentDescription.format = FindDepthFormat(*physicalDevice);
		depthAttachmentDescription.samples = msaaSamples;
		depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChain->GetImageFormat();
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = {
			colorAttachmentDescription,
			depthAttachmentDescription,
			colorAttachmentResolve
		};

		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(*logicalDevice, &createInfo, nullptr, &renderPass);
		Check(result == VK_SUCCESS, "Failed to create render pass. Vulkan error: %d", result);

	}

	void VulkanRendererAPI::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutCreateInfo.pBindings = bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(*logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
		Check(result == VK_SUCCESS, "Failed to create descriptor set layout. Vulkan error: %d", result);

	}

	void VulkanRendererAPI::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = numSamples;
		imageCreateInfo.flags = 0;

		VkResult result = vkCreateImage(*logicalDevice, &imageCreateInfo, nullptr, &image);
		Check(result == VK_SUCCESS, "Failed to create image. Vulkan error: %d", result);

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(*logicalDevice, image, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = FindMemoryType(*physicalDevice,
			memoryRequirements.memoryTypeBits,
			properties);

		result = vkAllocateMemory(*logicalDevice, &memoryAllocateInfo, nullptr, &imageMemory);
		Check(result == VK_SUCCESS, "Failed to allocate image memory. Vulkan error: %d", result);

		vkBindImageMemory(*logicalDevice, image, imageMemory, 0);
	}

	void VulkanRendererAPI::CreateColorResources()
	{
		VkFormat colorFormat = swapChain->GetImageFormat();

		CreateImage(swapChain->GetExtent().width, swapChain->GetExtent().height, 1,
			msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			colorImage, colorImageMemory);

		colorImageView = CreateImageView(*logicalDevice, colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void VulkanRendererAPI::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat(*physicalDevice);
		CreateImage(swapChain->GetExtent().width, swapChain->GetExtent().height, 1,
			msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage, depthImageMemory);

		depthImageView = CreateImageView(*logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}

	void VulkanRendererAPI::CreateTextureImage(const Texture& texture)
	{
		VkDeviceSize imageSize = texture.GetWidth() * texture.GetHeight() * 4;
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture.GetWidth(), texture.GetHeight())))) + 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		{
			memcpy(data, texture.Data(), static_cast<size_t>(imageSize));
		}
		vkUnmapMemory(*logicalDevice, stagingBufferMemory);

		CreateImage(texture.GetWidth(), texture.GetHeight(), mipLevels,
			VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage, textureImageMemory);

		TransitionImageLayout(logicalDevice, command->GetPool(),
			textureImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

		CopyBufferToImage(logicalDevice, command->GetPool(),
			stagingBuffer, textureImage,
			texture.GetWidth(), texture.GetHeight());

		/*TransitionImageLayout(logicalDevice, commandPool,
			textureImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);*/
		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);

		GenerateMipmaps(*physicalDevice, logicalDevice, command->GetPool(), textureImage, VK_FORMAT_R8G8B8A8_SRGB, texture.GetWidth(), texture.GetHeight(), mipLevels);
	}

	void VulkanRendererAPI::CreateTextureImageView()
	{
		textureImageView = CreateImageView(*logicalDevice, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	void VulkanRendererAPI::CreateTextureSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(*physicalDevice, &properties);

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = static_cast<float>(mipLevels);

		VkResult result = vkCreateSampler(*logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
		Check(result == VK_SUCCESS, "Failed to create texture sampler. Vulkan error: %d", result);
	}

	void VulkanRendererAPI::LoadModel()
	{
		//model = Model::GenerateQuad();
		//Texture texture = Texture("texture/texture.jpg");

		model = new Model("model/viking_room/viking_room.obj");
		model->SetTexture(Texture("model/viking_room/viking_room.png"));
	}

	void VulkanRendererAPI::CreateBuffer(
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
		allocateInfo.memoryTypeIndex = FindMemoryType(*physicalDevice, memoryRequirements.memoryTypeBits, properties);

		// NOTE: shouldn't do allocation for each individual buffer, instead should create a custom allocator
		// ref: https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer#:~:text=more%20complex%20geometry.-,Conclusion,-It%20should%20be
		result = vkAllocateMemory(*logicalDevice, &allocateInfo, nullptr, &bufferMemory);
		Check(result == VK_SUCCESS, "Failed to allocate vertex buffer memory. Vulkan error: %d", result);

		vkBindBufferMemory(*logicalDevice, buffer, bufferMemory, 0);
	}

	void VulkanRendererAPI::CreateVertexBuffer()
	{
		std::vector<Vertex> vertices = model->GetMesh(0)->GetVertices();
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		{
			memcpy(data, vertices.data(), (size_t)bufferSize);
		}
		vkUnmapMemory(*logicalDevice, stagingBufferMemory);

		CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer, vertexBufferMemory);

		CopyBuffer(logicalDevice, command->GetPool(), stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	}

	void VulkanRendererAPI::CreateIndexBuffer()
	{
		std::vector<uint32_t> indices = model->GetMesh(0)->GetIndices();
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		{
			memcpy(data, indices.data(), (size_t)bufferSize);
		}
		vkUnmapMemory(*logicalDevice, stagingBufferMemory);

		CreateBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer, indexBufferMemory);

		CopyBuffer(logicalDevice, command->GetPool(), stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	}

	void VulkanRendererAPI::CreateUniformBuffer()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(*logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}

	void VulkanRendererAPI::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkResult result = vkCreateDescriptorPool(*logicalDevice, &createInfo, nullptr, &descriptorPool);
		Check(result == VK_SUCCESS, "Failed to create descriptor pool. Vulkan error: %d", result);
	}

	void VulkanRendererAPI::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocateInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult result = vkAllocateDescriptorSets(*logicalDevice, &allocateInfo, descriptorSets.data());
		Check(result == VK_SUCCESS, "Failed to allocate descriptor sets. Vulkan error: %d", result);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = descriptorSets[i];
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].dstArrayElement = 0;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[0].pBufferInfo = &bufferInfo;

			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].dstSet = descriptorSets[i];
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].dstArrayElement = 0;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(*logicalDevice,
				static_cast<uint32_t>(writeDescriptorSets.size()),
				writeDescriptorSets.data(),
				0, nullptr);
		}
	}

	void VulkanRendererAPI::CreateSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult result = vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
			Check(result == VK_SUCCESS, "Failed to create image available semaphore. Vulkan error: %d", result);

			result = vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
			Check(result == VK_SUCCESS, "Failed to create image render finished semaphore. Vulkan error: %d", result);

			result = vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]);
			Check(result == VK_SUCCESS, "Failed to create in-flight fence. Vulkan error: %d", result);
		}
	}

	void VulkanRendererAPI::CleanUpSwapChain()
	{
		vkDestroyImageView(*logicalDevice, colorImageView, nullptr);
		vkDestroyImage(*logicalDevice, colorImage, nullptr);
		vkFreeMemory(*logicalDevice, colorImageMemory, nullptr);

		vkDestroyImageView(*logicalDevice, depthImageView, nullptr);
		vkDestroyImage(*logicalDevice, depthImage, nullptr);
		vkFreeMemory(*logicalDevice, depthImageMemory, nullptr);
	}

	void VulkanRendererAPI::RecreateSwapChain()
	{
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(nativeWindow, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(*logicalDevice);

		physicalDevice->Refresh(vulkanInstance);

		CleanUpSwapChain();

		swapChain->Recreate(vulkanInstance, physicalDevice, nativeWindow);

		CreateColorResources();
		CreateDepthResources();

		swapChain->CreateFrameBuffers(colorImageView, depthImageView, renderPass);
	}

	void VulkanRendererAPI::UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0, 0, 1));
		ubo.view = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
		ubo.projection = glm::perspective(glm::radians(45.0f), swapChain->GetExtent().width / (float)swapChain->GetExtent().height, 0.1f, 10.0f);
		ubo.projection[1][1] *= -1; // flip y-axis

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void VulkanRendererAPI::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		Check(result == VK_SUCCESS, "Failed to begin recording command buffer. Vulkan error: %d", result);

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = clearColor;
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapChain->GetFrameBuffer(imageIndex);
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.renderArea.extent = swapChain->GetExtent();
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline);

			VkViewport viewport{};
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = static_cast<float>(swapChain->GetExtent().width);
			viewport.height = static_cast<float>(swapChain->GetExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0,0 };
			scissor.extent = swapChain->GetExtent();

			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->GetLayout(), 0,
				1, &descriptorSets[currentFrame],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->GetMesh(0)->GetIndices().size()), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(commandBuffer);

		result = vkEndCommandBuffer(commandBuffer);
		Check(result == VK_SUCCESS, "Failed to record command buffer. Vulkan error: %d", result);
	}
#pragma endregion

}