#pragma once

#include <sstream>
#include <string>

#define EVENT_CLASS_TYPE(type) \
    static EWindowEventType GetStaticType() { return EWindowEventType::type; } \
    virtual EWindowEventType GetEventType() const override { return GetStaticType(); } \
    virtual const char* GetName() const override { return #type; }

namespace FGEngine
{
    enum class EWindowEventType : short
    {
        WindowResize,
        WindowClose,
        WindowFocusChanged,

        CursorPosition,
        CursorEnterChanged,
        MousePressed,
        MouseReleased,
        MouseScrolled,

        KeyPressed,
        KeyReleased,
        KeyRepeated,
    };

    class IWindowEvent
    {
    public:
        virtual EWindowEventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }
    };

    class WindowResizeEvent : public IWindowEvent
    {
    public:
        EVENT_CLASS_TYPE(WindowResize);

        WindowResizeEvent(int width, int height)
            : width(width)
            , height(height)
        {

        }

        int GetWidth() { return width; }
        int GetHeight() { return height; }

        virtual std::string ToString() const override
        {
            std::stringstream ss;
            ss << GetName() << ": " << width << " x " << height;
            return ss.str();
        }

    private:
        int width;
        int height;
    };

	class WindowClosedEvent : public IWindowEvent
	{
        EVENT_CLASS_TYPE(WindowClose);
	};

    class WindowFocusChangedEvent : public IWindowEvent
    {
    public:
        EVENT_CLASS_TYPE(WindowFocusChanged);

        WindowFocusChangedEvent(bool isFocused)
            : bIsFocused(isFocused)
        {

        }

        int GetFocused() { return bIsFocused; }

        virtual std::string ToString() const override
        {
            std::stringstream ss;
            ss << GetName() << ": " << bIsFocused;
            return ss.str();
        }

    private:
        bool bIsFocused;
    };
}

