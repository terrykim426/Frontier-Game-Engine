#pragma once

#include "core/AppLayerStack.h"
#include "core/Window.h"

#include <memory>


int main(int argc, char** argv);

namespace FGEngine
{
	class Application
	{
		friend int ::main(int argc, char** argv);

	public:
		ENGINE_API Application();
		ENGINE_API virtual ~Application();

	private:
		ENGINE_API void Run();

	public:
		bool CanClose();
		void Close();

		ENGINE_API void PushLayer(AppLayer* appLayer);
		ENGINE_API void PopLayer(AppLayer* appLayer);
		ENGINE_API void PushOverlay(AppLayer* appLayer);
		ENGINE_API void PopOverlay(AppLayer* appLayer);

	private:
		void OnWindowEvent(const std::shared_ptr<IWindowEvent>& windowEvent);

	private:
		bool bIsRunning;

		std::unique_ptr<IWindow> window;
		AppLayerStack layerStack;

		class InputSubsystem* inputSubsystem;
		class RendererSubsystem* rendererSubsystem;
	};

	// to be defined in client
	Application* CreateApplication();
}