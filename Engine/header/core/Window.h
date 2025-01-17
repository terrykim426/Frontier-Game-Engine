#pragma once

#include <string>

#include "core/Delegate.h"
#include "event/WindowEvent.h"

DECLARE_DELEGATE(Window, const std::shared_ptr<FGEngine::IWindowEvent>&);

namespace FGEngine
{
	struct WindowProperties
	{
	public:
		WindowProperties(const std::string& title = "Frontier Game Engine", 
						 unsigned int width = 640, 
						 unsigned int height = 480) :
			title(title),
			width(width),
			height(height)
		{
		}

	public:
		std::string title;
		unsigned int width;
		unsigned int height;
	};

	class IWindow
	{
	public:
		virtual ~IWindow() = default;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void OnUpdate(float deltaTime) = 0;
		virtual void SetVSync(bool bEnable) = 0;
		virtual bool IsVSync() = 0;

		WindowDelegate windowDelegate;

		static IWindow* Create(const WindowProperties& properties = WindowProperties());
	};
}