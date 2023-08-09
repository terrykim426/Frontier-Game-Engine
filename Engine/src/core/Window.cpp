#include "pch.h"
#include "core/Window.h"
#include "core/Logger.h"

#if _WIN32
#include "platform/WindowsWindow.h"
#endif

namespace FGEngine
{
	IWindow* IWindow::Create(const WindowProperties& properties)
	{
#if _WIN32
		return new WindowsWindow(properties);
#else
		LogError("Unsupported platform");
		return nullptr;
#endif
	}
}