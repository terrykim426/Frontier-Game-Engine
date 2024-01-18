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
#include "platform/vulkan/VulkanImageView.h"
#include "platform/vulkan/VulkanTextureImageView.h"
#include "platform/vulkan/VulkanBuffer.h"
#include "platform/vulkan/VulkanDescriptor.h"
#include "platform/vulkan/VulkanUtil.h"

#include "core/Logger.h"
#include "renderer/Texture.h"
#include "renderer/Model.h"
#include "renderer/Shader.h"

#include <chrono>

#include <glm/gtc/matrix_transform.hpp>


namespace FGEngine
{
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

		descriptor = std::make_shared<VulkanDescriptor>(logicalDevice, MAX_FRAMES_IN_FLIGHT);

		Shader vertShader("shader/TestShaderVert.spv", Shader::EType::Vertex);
		Shader fragShader("shader/TestShaderFrag.spv", Shader::EType::Fragment);

		VulkanPipelineSetting pipelineSetting;
		pipelineSetting.vertexShaderModule = std::make_shared<VulkanShaderModule>(logicalDevice, vertShader);
		pipelineSetting.fragmentShaderModule = std::make_shared<VulkanShaderModule>(logicalDevice, fragShader);
		pipelineSetting.msaaSamples = msaaSamples;
		pipelineSetting.descriptorSetLayout = descriptor->GetSetLayout();
		pipelineSetting.renderPass = renderPass;

		graphicsPipeline = std::make_shared<VulkanPipeline>(logicalDevice, pipelineSetting);

		CreateColorResources();
		CreateDepthResources();

		swapChain->CreateFrameBuffers(*colorImageView, *depthImageView, renderPass);

		command = std::make_shared<VulkanCommand>(physicalDevice, logicalDevice, MAX_FRAMES_IN_FLIGHT);

		LoadModel();

		CreateUniformBuffer();

		descriptor->CreateDescriptorSets(MAX_FRAMES_IN_FLIGHT, uniformBuffers, sizeof(UniformBufferObject), textureImageView);

		CreateSyncObjects();
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
		vkDeviceWaitIdle(*logicalDevice);

		VulkanUtil::VectorDestroy(vkDestroySemaphore, *logicalDevice, imageAvailableSemaphores, nullptr);
		VulkanUtil::VectorDestroy(vkDestroySemaphore, *logicalDevice, renderFinishedSemaphores, nullptr);
		VulkanUtil::VectorDestroy(vkDestroyFence, *logicalDevice, inFlightFences, nullptr);

		if (model)
		{
			delete model;
			model = nullptr;
		}

		VulkanUtil::VectorDestroy(vkDestroyBuffer, *logicalDevice, uniformBuffers, nullptr);
		VulkanUtil::VectorDestroy(vkFreeMemory, *logicalDevice, uniformBuffersMemory, nullptr);


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
		depthAttachmentDescription.format = physicalDevice->FindDepthFormat();
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

	void VulkanRendererAPI::CreateColorResources()
	{
		VulkanImageViewSetting colorImageViewSetting{};
		colorImageViewSetting.imageFormat = swapChain->GetImageFormat();
		colorImageViewSetting.msaaSamples = msaaSamples;
		colorImageViewSetting.imageUsageFlags = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		colorImageViewSetting.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		colorImageView.reset();
		colorImageView = std::make_shared<VulkanImageView>(physicalDevice, logicalDevice, swapChain, colorImageViewSetting);
	}

	void VulkanRendererAPI::CreateDepthResources()
	{
		VulkanImageViewSetting depthImageViewSetting{};
		depthImageViewSetting.imageFormat = physicalDevice->FindDepthFormat();
		depthImageViewSetting.msaaSamples = msaaSamples;
		depthImageViewSetting.imageUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageViewSetting.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

		depthImageView.reset();
		depthImageView = std::make_shared<VulkanImageView>(physicalDevice, logicalDevice, swapChain, depthImageViewSetting);
	}

	void VulkanRendererAPI::LoadModel()
	{
		//model = Model::GenerateQuad();
		//Texture texture = Texture("texture/texture.jpg");

		model = new Model("model/viking_room/viking_room.obj");
		model->SetTexture(Texture("model/viking_room/viking_room.png"));

		textureImageView = std::make_shared<VulkanTextureImageView>(
			physicalDevice, logicalDevice,
			swapChain, command,
			*model->GetTexture());


		std::vector<Vertex> vertices = model->GetMesh(0)->GetVertices();
		vertexBuffer = std::make_shared<VulkanBuffer>(logicalDevice);
		vertexBuffer->Init(physicalDevice, command, vertices, EBufferType::Vertex);

		std::vector<uint32_t> indices = model->GetMesh(0)->GetIndices();
		indexBuffer = std::make_shared<VulkanBuffer>(logicalDevice);
		indexBuffer->Init(physicalDevice, command, indices, EBufferType::Index);
	}

	void VulkanRendererAPI::CreateUniformBuffer()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VulkanBuffer::CreateBuffer(
				physicalDevice, logicalDevice,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(*logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
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

		swapChain->Recreate(vulkanInstance, physicalDevice, nativeWindow);

		CreateColorResources();
		CreateDepthResources();

		swapChain->CreateFrameBuffers(*colorImageView, *depthImageView, renderPass);
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

			VkBuffer vertexBuffers[] = { *vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, *indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			VkDescriptorSet descriptorSet = descriptor->GetSet(currentFrame);
			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->GetLayout(), 0,
				1, &descriptorSet,
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->GetMesh(0)->GetIndices().size()), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(commandBuffer);

		result = vkEndCommandBuffer(commandBuffer);
		Check(result == VK_SUCCESS, "Failed to record command buffer. Vulkan error: %d", result);
	}
#pragma endregion

}