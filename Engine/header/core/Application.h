#pragma once

#include "core/AppLayerStack.h"
#include "core/Window.h"

#include <memory>


int main(int argc, char** argv);

namespace FGEngine
{
	class ENGINE_API Application
	{
		friend int ::main(int argc, char** argv);

	public:
		Application();
		virtual ~Application();

	private:
		void Run();

	public:
		bool CanClose();
		void Close();

		void PushLayer(AppLayer* appLayer);
		void PopLayer(AppLayer* appLayer);
		void PushOverlay(AppLayer* appLayer);
		void PopOverlay(AppLayer* appLayer);

	private:
		void OnWindowEvent(const std::shared_ptr<IWindowEvent>& windowEvent);

	private:
		bool bIsRunning;
#pragma warning (suppress : 4251)
		std::unique_ptr<IWindow> window;
		AppLayerStack layerStack;
	};

	// to be defined in client
	Application* CreateApplication();
}