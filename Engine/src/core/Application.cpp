#include "pch.h"
#include "core/Application.h"
#include "core/AppLayer.h"

namespace FGEngine
{
	Application::Application()
	{
		bIsRunning = true;
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		for (AppLayer* appLayer : layerStack)
		{
			appLayer->OnUpdate(0);
		}
	}

	bool Application::CanClose()
	{
		return bIsRunning;
	}

	void Application::Close()
	{
		bIsRunning = false;
	}

	void Application::PushLayer(AppLayer* appLayer)
	{
		layerStack.PushLayer(appLayer);
	}

	void Application::PopLayer(AppLayer* appLayer)
	{
		layerStack.PopLayer(appLayer);
	}

	void Application::PushOverlay(AppLayer* appLayer)
	{
		layerStack.PushOverlay(appLayer);
	}

	void Application::PopOverlay(AppLayer* appLayer)
	{
		layerStack.PopOverlay(appLayer);
	}
}