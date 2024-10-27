#pragma once

#include "event/KeyboardEvent.h"
#include "subsystem/EngineSubsystem.h"
#include "core/Delegate.h"

#include <glm/glm.hpp>
#include <vector>

namespace FGEngine
{
enum class EKey : uint8_t
{
	Key_Unknown,
	Key_Space,
	Key_Apostrophe,
	Key_Comma,
	Key_Minus,
	Key_Period,
	Key_Slash,
	Key_0,
	Key_1,
	Key_2,
	Key_3,
	Key_4,
	Key_5,
	Key_6,
	Key_7,
	Key_8,
	Key_9,
	Key_Semicolon,
	Key_Equal,
	Key_A,
	Key_B,
	Key_C,
	Key_D,
	Key_E,
	Key_F,
	Key_G,
	Key_H,
	Key_I,
	Key_J,
	Key_K,
	Key_L,
	Key_M,
	Key_N,
	Key_O,
	Key_P,
	Key_Q,
	Key_R,
	Key_S,
	Key_T,
	Key_U,
	Key_V,
	Key_W,
	Key_X,
	Key_Y,
	Key_Z,
	Key_Left_Bracket,
	Key_Backslash,
	Key_Right_Bracket,
	Key_Grave_Accent,
	Key_World_1,
	Key_World_2,
	Key_Escape,
	Key_Enter,
	Key_Tab,
	Key_Backspace,
	Key_Insert,
	Key_Delete,
	Key_Right,
	Key_Left,
	Key_Down,
	Key_Up,
	Key_Page_Up,
	Key_Page_Down,
	Key_Home,
	Key_End,
	Key_Caps_Lock,
	Key_Scroll_Lock,
	Key_Num_Lock,
	Key_Print_Screen,
	Key_Pause,
	Key_F1,
	Key_F2,
	Key_F3,
	Key_F4,
	Key_F5,
	Key_F6,
	Key_F7,
	Key_F8,
	Key_F9,
	Key_F10,
	Key_F11,
	Key_F12,
	Key_F13,
	Key_F14,
	Key_F15,
	Key_F16,
	Key_F17,
	Key_F18,
	Key_F19,
	Key_F20,
	Key_F21,
	Key_F22,
	Key_F23,
	Key_F24,
	Key_F25,
	Key_Numpad_0,
	Key_Numpad_1,
	Key_Numpad_2,
	Key_Numpad_3,
	Key_Numpad_4,
	Key_Numpad_5,
	Key_Numpad_6,
	Key_Numpad_7,
	Key_Numpad_8,
	Key_Numpad_9,
	Key_Numpad_Decimal,
	Key_Numpad_Divide,
	Key_Numpad_Multiply,
	Key_Numpad_Subtract,
	Key_Numpad_Add,
	Key_Numpad_Enter,
	Key_Numpad_Equal,
	Key_Left_Shift,
	Key_Left_Control,
	Key_Left_Alt,
	Key_Left_Super,
	Key_Right_Shift,
	Key_Right_Control,
	Key_Right_Alt,
	Key_Right_Super,
	Key_Menu,

	Count
};

enum class EKeyState : uint8_t
{
	Unknown,
	Pressed,
	Released,
	Repeated,
};

enum class EMouseButton : uint8_t
{
	Mouse_Unknown,
	Mouse_Left,
	Mouse_Right,
	Mouse_Center,
	Mouse_Button4,
	Mouse_Button5,
	Mouse_Button6,
	Mouse_Button7,
	Mouse_Button8,

	Count
};

DECLARE_DELEGATE(InputKey, EKey, EKeyState);
DECLARE_DELEGATE(InputMouse, EMouseButton, EKeyState);
DECLARE_DELEGATE(InputMouseScroll, const glm::dvec2&);
DECLARE_DELEGATE(InputCursorPosition, const glm::dvec2&);
DECLARE_DELEGATE(InputCursorMove, const glm::dvec2&);
DECLARE_DELEGATE(InputCursorEnterChanged, bool);

class InputSubsystem : public EngineSubsystem
{
public:
	InputSubsystem()
		: keyStates((size_t)EKey::Count)
		, mouseStates((size_t)EMouseButton::Count)
	{
	}

	void ProcessQueue();
	void AddQueue(const std::shared_ptr<IWindowEvent>& windowEvent);

	bool IsKeyPressed(EKey key) const;
	bool IsKeyReleased(EKey key) const;
	bool IsKeyRepeated(EKey key) const;

	bool IsMouseButtonPressed(EMouseButton button) const;
	bool IsMouseButtonReleased(EMouseButton button) const;

	glm::dvec2 GetMouseScroll() const;
	glm::dvec2 GetCursorPosition() const;
	glm::dvec2 GetCursorDelta() const;

public:
	InputKeyDelegate inputKeyDelegate;
	InputMouseDelegate inputMouseDelegate;
	InputMouseScrollDelegate inputMouseScrollDelegate;
	InputCursorPositionDelegate inputCursorPositionDelegate;
	InputCursorMoveDelegate inputCursorMoveDelegate;
	InputCursorEnterChangedDelegate inputCursorEnterChangedDelegate;

private:
	std::vector<std::shared_ptr<IWindowEvent>> queueEvents;
	std::vector<EKeyState> keyStates;
	std::vector<EKeyState> mouseStates;
	glm::dvec2 mouseScroll;
	glm::dvec2 cursorPosition;
	glm::dvec2 cursorPreviousPosition;
	glm::dvec2 cursorDelta;
};

}