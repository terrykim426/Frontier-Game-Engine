#include "pch.h"
#include "platform/WindowsWindow.h"
#include "core/Logger.h"
#include "event/MouseEvent.h"
#include "event/KeyboardEvent.h"

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

		if (!windowEventHandle.IsValid())
		{
			windowEventHandle = windowData.windowDelegate->Add1(this, WindowsWindow::OnWindowEvent);
		}

		nativeWindow = glfwCreateWindow(windowData.width, windowData.height, windowData.title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(nativeWindow);
		glfwSetWindowUserPointer(nativeWindow, &windowData);

		SetVSync(true);

		glfwSetWindowSizeCallback(nativeWindow, [](GLFWwindow* glWindow, int width, int height)
								  {
									  auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
									  data->width = width;
									  data->height = height;
									  data->windowDelegate->Broadcast(WindowResizeEvent(width, height));
								  });

		glfwSetWindowCloseCallback(nativeWindow, [](GLFWwindow* window)
								   {
									   auto* data = (WindowData*)glfwGetWindowUserPointer(window);
									   data->windowDelegate->Broadcast(WindowClosedEvent());
								   });

		glfwSetWindowFocusCallback(nativeWindow, [](GLFWwindow* glWindow, int focused)
								   {
									   auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
									   data->windowDelegate->Broadcast(WindowFocusChangedEvent(focused));
								   });


		glfwSetCursorPosCallback(nativeWindow, [](GLFWwindow* glWindow, double xpos, double ypos)
								 {
									 auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
									 data->windowDelegate->Broadcast(CursorPositionEvent(xpos, ypos));
								 });

		glfwSetCursorEnterCallback(nativeWindow, [](GLFWwindow* glWindow, int entered)
								   {
									   auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
									   data->windowDelegate->Broadcast(CursorEnterChangedEvent(entered));
								   });

		glfwSetMouseButtonCallback(nativeWindow, [](GLFWwindow* glWindow, int button, int action, int mods)
								   {
									   auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
									   switch (action)
									   {
									   case GLFW_PRESS:
										   data->windowDelegate->Broadcast(MousePressedEvent(button, mods));
										   break;
									   case GLFW_RELEASE:
										   data->windowDelegate->Broadcast(MouseReleasedEvent(button, mods));
										   break;
									   }
								   });

		glfwSetScrollCallback(nativeWindow, [](GLFWwindow* glWindow, double xoffset, double yoffset)
							  {
								  auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
								  data->windowDelegate->Broadcast(MouseScrolledEvent(xoffset, yoffset));
							  });

		glfwSetKeyCallback(nativeWindow, [](GLFWwindow* glWindow, int key, int scancode, int action, int mods)
						   {
							   auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
							   switch (action)
							   {
							   case GLFW_PRESS:
								   data->windowDelegate->Broadcast(KeyPressedEvent(key, mods));
								   break;
							   case GLFW_RELEASE:
								   data->windowDelegate->Broadcast(KeyReleasedEvent(key, mods));
								   break;
							   case GLFW_REPEAT:
								   data->windowDelegate->Broadcast(KeyRepeatedEvent(key, mods));
								   break;
							   }
						   });
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(nativeWindow);
	}

	void WindowsWindow::OnWindowEvent(const IWindowEvent& windowEvent)
	{
		windowDelegate->Broadcast(windowEvent);
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