#pragma once

#include "core/AppLayerStack.h"


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
		bool bIsRunning;
		AppLayerStack layerStack;
	};

	// to be defined in client
	Application* CreateApplication();
}