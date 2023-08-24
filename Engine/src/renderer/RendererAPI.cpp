#include "pch.h"
#include "renderer/RendererAPI.h"

#include "core/Logger.h"
#include "platform/openGL/OpenGLRendererAPI.h"
#include "platform/vulkan/VulkanRendererAPI.h"

namespace FGEngine
{
	IRendererAPI* IRendererAPI::Create(const RendererProperties& rendererProperties)
	{
		ERendererAPI api = rendererProperties.rendererAPI;
		switch (api)
		{
		case ERendererAPI::OpenGL:
			if (OpenGLRendererAPI::IsSupported())
			{
				return new OpenGLRendererAPI();
			}

			LogWarning("OpenGL is not supported");
			break;
		case ERendererAPI::Vulkan:
			if (VulkanRendererAPI::IsSupported())
			{
				return new VulkanRendererAPI(rendererProperties);
			}

			LogWarning("Vulkan is not supported");
			break;
		}

		NoEntry("Selected Renderer API not supported!");

		return nullptr;
	}
}