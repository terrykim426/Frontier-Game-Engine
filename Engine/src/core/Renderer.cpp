#include "pch.h"
#include "renderer/Renderer.h"
#include "renderer/RendererAPI.h"

#include "core/Logger.h"

namespace FGEngine
{
	std::unique_ptr<IRendererAPI> Renderer::s_api;

	void Renderer::Init(const RendererProperties& rendererProperties)
	{
		s_api = std::unique_ptr<IRendererAPI>(IRendererAPI::Create(rendererProperties));
		if (s_api.get())
		{
			LogInfo("Renderer API Loaded: %s", s_api->GetName().c_str());
			LogInfo("Renderer API Version: %s", s_api->GetVersion().c_str());
		}
	}

	void Renderer::Shutdown()
	{
		s_api.reset();
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{
		s_api->SetClearColor(r, g, b, a);
	}

	void Renderer::Clear()
	{
		s_api->Clear();
	}

	void Renderer::Render(void* nativeWindow)
	{
		s_api->Render(nativeWindow);
	}
	void Renderer::Resize()
	{
		s_api->Resize();
	}
}
