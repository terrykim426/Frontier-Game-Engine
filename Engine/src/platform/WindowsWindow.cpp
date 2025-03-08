#include "pch.h"
#include "platform/WindowsWindow.h"
#include "core/Logger.h"
#include "event/MouseEvent.h"
#include "event/KeyboardEvent.h"

#include <iostream>
#include <memory>

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

	windowData.windowDelegate.AddFunction(this, WindowsWindow::OnWindowEvent);

	if (windowProperties.bNoAPI)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	nativeWindow = glfwCreateWindow(windowData.width, windowData.height, windowData.title.c_str(), nullptr, nullptr);

	if (!windowProperties.bNoAPI)
	{
		glfwMakeContextCurrent(nativeWindow);
	}
	glfwSetWindowUserPointer(nativeWindow, &windowData);

	SetVSync(true);

	glfwSetWindowSizeCallback(nativeWindow, [](GLFWwindow* glWindow, int width, int height)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			data->width = width;
			data->height = height;
			data->windowDelegate.Broadcast(std::make_shared<WindowResizeEvent>(width, height));
		});

	glfwSetWindowCloseCallback(nativeWindow, [](GLFWwindow* window)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(window);
			data->windowDelegate.Broadcast(std::make_shared<WindowClosedEvent>());
		});

	glfwSetWindowFocusCallback(nativeWindow, [](GLFWwindow* glWindow, int focused)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			data->windowDelegate.Broadcast(std::make_shared<WindowFocusChangedEvent>(focused));
		});


	glfwSetCursorPosCallback(nativeWindow, [](GLFWwindow* glWindow, double xpos, double ypos)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			data->windowDelegate.Broadcast(std::make_shared<CursorPositionEvent>(xpos, ypos));
		});

	glfwSetCursorEnterCallback(nativeWindow, [](GLFWwindow* glWindow, int entered)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			data->windowDelegate.Broadcast(std::make_shared<CursorEnterChangedEvent>(entered));
		});

	glfwSetMouseButtonCallback(nativeWindow, [](GLFWwindow* glWindow, int button, int action, int mods)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			switch (action)
			{
			case GLFW_PRESS:
				data->windowDelegate.Broadcast(std::make_shared<MousePressedEvent>(button, mods));
				break;
			case GLFW_RELEASE:
				data->windowDelegate.Broadcast(std::make_shared<MouseReleasedEvent>(button, mods));
				break;
			}
		});

	glfwSetScrollCallback(nativeWindow, [](GLFWwindow* glWindow, double xoffset, double yoffset)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			data->windowDelegate.Broadcast(std::make_shared<MouseScrolledEvent>(xoffset, yoffset));
		});

	glfwSetKeyCallback(nativeWindow, [](GLFWwindow* glWindow, int key, int scancode, int action, int mods)
		{
			auto* data = (WindowData*)glfwGetWindowUserPointer(glWindow);
			switch (action)
			{
			case GLFW_PRESS:
				data->windowDelegate.Broadcast(std::make_shared<KeyPressedEvent>(key, mods));
				break;
			case GLFW_RELEASE:
				data->windowDelegate.Broadcast(std::make_shared<KeyReleasedEvent>(key, mods));
				break;
			case GLFW_REPEAT:
				data->windowDelegate.Broadcast(std::make_shared<KeyRepeatedEvent>(key, mods));
				break;
			}
		});
}

void WindowsWindow::Shutdown()
{
	windowData.windowDelegate.RemoveFunction(this, WindowsWindow::OnWindowEvent);
	glfwDestroyWindow(nativeWindow);
	glfwTerminate();
}

void WindowsWindow::OnWindowEvent(const std::shared_ptr<IWindowEvent>& windowEvent)
{
	windowDelegate.Broadcast(windowEvent);
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
	glfwPollEvents();
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