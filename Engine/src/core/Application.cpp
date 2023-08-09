#include "pch.h"
#include "core/Application.h"
#include "core/AppLayer.h"

namespace FGEngine
{
	Application::Application()
	{
		bIsRunning = true;
		window = std::unique_ptr<IWindow>(IWindow::Create());
		windowEventHandle = window->windowDelegate->Add1(this, Application::OnWindowEvent);
	}

	Application::~Application()
	{
		window.reset();
	}

	void Application::Run()
	{
		while (bIsRunning)
		{
			for (AppLayer* appLayer : layerStack)
			{
				appLayer->OnUpdate(0);
			}

			window->OnUpdate(0);
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

	void Application::OnWindowEvent(const IWindowEvent& windowEvent)
	{
		LogInfo("Window Event (%s)", windowEvent.ToString().c_str());
		switch (windowEvent.GetEventType())
		{
		case EWindowEventType::WindowClose:
		{
			bIsRunning = false;
		}
		break;
		}
	}
}