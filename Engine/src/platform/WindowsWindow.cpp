#include "pch.h"
#include "platform/WindowsWindow.h"
#include "core/Logger.h"

#include <iostream>

namespace FGEngine
{
	bool WindowsWindow::bIsInitialized = false;

	WindowsWindow::WindowsWindow(const WindowProperties& windowProperties)
	{
		Init(windowProperties);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProperties& windowProperties)
	{
		windowData.title = windowProperties.title;
		windowData.width = windowProperties.width;
		windowData.height = windowProperties.height;

		if (!bIsInitialized)
		{
			int success = glfwInit();
			Check(success, "GLFW failed to initialize");
			bIsInitialized = true;
		}

		nativeWindow = glfwCreateWindow(windowData.width, windowData.height, windowData.title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(nativeWindow);
		glfwSetWindowUserPointer(nativeWindow, &windowData);

		SetVSync(true);


	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(nativeWindow);
	}

	unsigned int WindowsWindow::GetWidth() const
	{
		return windowData.width;
	}

	unsigned int WindowsWindow::GetHeight() const
	{
		return windowData.height;
	}

	void WindowsWindow::OnUpdate(float deltaTime)
	{
		glClearColor(1, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwPollEvents();
		glfwSwapBuffers(nativeWindow);
	}

	void WindowsWindow::SetVSync(bool bEnable)
	{
		glfwSwapInterval(bEnable ? 1 : 0);
		windowData.bVSync = bEnable;
	}

	bool WindowsWindow::IsVSync()
	{
		return windowData.bVSync;
	}

}