#pragma once

#include "event/WindowEvent.h"

namespace FGEngine
{
	class CursorPositionEvent : public IWindowEvent
	{
	public:
		EVENT_CLASS_TYPE(CursorPosition);

		CursorPositionEvent(double xPosition, double yPosition)
			: x(xPosition)
			, y(yPosition)
		{
		}

		double GetPositionX() { return x; }
		double GetPositionY() { return y; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << x << " x " << y;
			return ss.str();
		}

	private:
		double x;
		double y;
	};

	class CursorEnterChangedEvent : public IWindowEvent
	{
	public:
		EVENT_CLASS_TYPE(CursorEnterChanged);

		CursorEnterChangedEvent(bool isEntered)
			: bIsEntered(isEntered)
		{
		}

		bool IsEntered() { return bIsEntered; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << bIsEntered;
			return ss.str();
		}

	private:
		bool bIsEntered;
	};

	class MouseButtonEvent : public IWindowEvent
	{
	public:
		MouseButtonEvent(int button, int mods)
			: button(button)
			, mods(mods)
		{
		}

		int GetButton() { return button; }
		int GetMods() { return mods; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << button << " - " << mods;
			return ss.str();
		}

	private:
		int button;
		int mods;
	};

	class MousePressedEvent : public MouseButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(MousePressed);

		MousePressedEvent(int button, int mods) : MouseButtonEvent(button, mods)
		{
		}
	};

	class MouseReleasedEvent : public MouseButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseReleased);

		MouseReleasedEvent(int button, int mods) : MouseButtonEvent(button, mods)
		{
		}
	};

	class MouseScrolledEvent : public IWindowEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseScrolled);

		MouseScrolledEvent(double xOffset, double yOffset)
			: xOffset(xOffset)
			, yOffset(yOffset)
		{

		}

		double GetOffsetX() { return xOffset; }
		double GetOffsetY() { return yOffset; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << xOffset << " x " << yOffset;
			return ss.str();
		}

	private:
		double xOffset;
		double yOffset;
	};
} // namespace FGEngine