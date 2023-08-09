#pragma once

#include "core/Window.h"
#include "GLFW/glfw3.h"

namespace FGEngine
{
	class WindowsWindow : public IWindow
	{
	public:
		WindowsWindow(const WindowProperties& windowProperties);
		virtual ~WindowsWindow() override;

		// Inherited via IWindow
		virtual unsigned int GetWidth() const override;
		virtual unsigned int GetHeight() const override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void SetVSync(bool bEnable) override;
		virtual bool IsVSync() override;

	protected:
		virtual void Init(const WindowProperties& windowProperties);
		virtual void Shutdown();

	private:
		static bool bIsInitialized;
		GLFWwindow* nativeWindow;

		struct WindowData
		{
			std::string title;
			unsigned int width;
			unsigned int height;

			bool bVSync;
		};
		WindowData windowData;
	};
}

