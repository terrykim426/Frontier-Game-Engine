#include "pch.h"
#include "core/Application.h"
#include "core/AppLayer.h"
#include "core/InputSubsystem.h"
#include "subsystem/SubsystemManager.h"

namespace FGEngine
{
Application::Application()
{
	bIsRunning = true;
	window = std::unique_ptr<IWindow>(IWindow::Create());
	window->windowDelegate.AddFunction(this, Application::OnWindowEvent);

	inputSubsystem = SubsystemManager::Get().RegisterSubsystem<InputSubsystem>();
}

Application::~Application()
{
	SubsystemManager::Get().UnregisterSubsystem<InputSubsystem>();
	window->windowDelegate.RemoveFunction(this, Application::OnWindowEvent);
	window.reset();
}

void Application::Run()
{
	float deltaTime = 0;	// TODO: setup delta time
	while (bIsRunning)
	{
		for (AppLayer* appLayer : layerStack)
		{
			appLayer->OnUpdate(deltaTime);
		}

		window->OnUpdate(deltaTime);
		inputSubsystem->ProcessQueue();
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

void Application::OnWindowEvent(const std::shared_ptr<IWindowEvent>& windowEvent)
{
	LogInfo("Window Event (%s)", windowEvent->ToString().c_str());
	switch (windowEvent->GetEventType())
	{
	case EWindowEventType::WindowClose:
	{
		bIsRunning = false;
	}
	break;
	case EWindowEventType::CursorPosition:
	case EWindowEventType::CursorEnterChanged:
	case EWindowEventType::MousePressed:
	case EWindowEventType::MouseReleased:
	case EWindowEventType::MouseScrolled:
	case EWindowEventType::KeyPressed:
	case EWindowEventType::KeyReleased:
	case EWindowEventType::KeyRepeated:
	{
		inputSubsystem->AddQueue(windowEvent);
	}
	break;
	}
}
}