#include "pch.h"
#include "renderer/RendererSubsystem.h"
#include "renderer/RendererAPI.h"

#include "core/Logger.h"
#include "event/WindowEvent.h"

namespace FGEngine
{
RendererSubsystem::RendererSubsystem(const RendererProperties& rendererProperties)
{
	api = std::unique_ptr<IRendererAPI>(IRendererAPI::Create(rendererProperties));
	if (api.get())
	{
		LogInfo("Renderer API Loaded: %s", api->GetName().c_str());
		LogInfo("Renderer API Version: %s", api->GetVersion().c_str());
	}
}

RendererSubsystem::~RendererSubsystem()
{
	api.reset();
}

void RendererSubsystem::SetClearColor(float r, float g, float b, float a)
{
	api->SetClearColor(r, g, b, a);
}

void RendererSubsystem::Clear()
{
	api->Clear();
}

void RendererSubsystem::Render()
{
	api->Render(nullptr);
}

void RendererSubsystem::Resize(const std::shared_ptr<FGEngine::IWindowEvent>& windowEvent)
{
	switch (windowEvent->GetEventType())
	{
	case EWindowEventType::WindowResize:
		api->Resize();
		break;
	}
}
}
