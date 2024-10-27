#include "pch.h"
#include "core/InputSubsystem.h"
#include "core/Logger.h"
#include "event/KeyboardEvent.h"
#include "event/MouseEvent.h"

#include <GLFW/glfw3.h>

namespace FGEngine
{
#pragma region Helper
static EKey GLFWKeyToEKey(int16_t glfwKey)
{
	switch (glfwKey)
	{
	case GLFW_KEY_UNKNOWN: return EKey::Key_Unknown;
	case GLFW_KEY_SPACE: return EKey::Key_Space;
	case GLFW_KEY_APOSTROPHE: return EKey::Key_Apostrophe;
	case GLFW_KEY_COMMA: return EKey::Key_Comma;
	case GLFW_KEY_MINUS: return EKey::Key_Minus;
	case GLFW_KEY_PERIOD: return EKey::Key_Period;
	case GLFW_KEY_SLASH: return EKey::Key_Slash;
	case GLFW_KEY_0: return EKey::Key_0;
	case GLFW_KEY_1: return EKey::Key_1;
	case GLFW_KEY_2: return EKey::Key_2;
	case GLFW_KEY_3: return EKey::Key_3;
	case GLFW_KEY_4: return EKey::Key_4;
	case GLFW_KEY_5: return EKey::Key_5;
	case GLFW_KEY_6: return EKey::Key_6;
	case GLFW_KEY_7: return EKey::Key_7;
	case GLFW_KEY_8: return EKey::Key_8;
	case GLFW_KEY_9: return EKey::Key_9;
	case GLFW_KEY_SEMICOLON: return EKey::Key_Semicolon;
	case GLFW_KEY_EQUAL: return EKey::Key_Equal;
	case GLFW_KEY_A: return EKey::Key_A;
	case GLFW_KEY_B: return EKey::Key_B;
	case GLFW_KEY_C: return EKey::Key_C;
	case GLFW_KEY_D: return EKey::Key_D;
	case GLFW_KEY_E: return EKey::Key_E;
	case GLFW_KEY_F: return EKey::Key_F;
	case GLFW_KEY_G: return EKey::Key_G;
	case GLFW_KEY_H: return EKey::Key_H;
	case GLFW_KEY_I: return EKey::Key_I;
	case GLFW_KEY_J: return EKey::Key_J;
	case GLFW_KEY_K: return EKey::Key_K;
	case GLFW_KEY_L: return EKey::Key_L;
	case GLFW_KEY_M: return EKey::Key_M;
	case GLFW_KEY_N: return EKey::Key_N;
	case GLFW_KEY_O: return EKey::Key_O;
	case GLFW_KEY_P: return EKey::Key_P;
	case GLFW_KEY_Q: return EKey::Key_Q;
	case GLFW_KEY_R: return EKey::Key_R;
	case GLFW_KEY_S: return EKey::Key_S;
	case GLFW_KEY_T: return EKey::Key_T;
	case GLFW_KEY_U: return EKey::Key_U;
	case GLFW_KEY_V: return EKey::Key_V;
	case GLFW_KEY_W: return EKey::Key_W;
	case GLFW_KEY_X: return EKey::Key_X;
	case GLFW_KEY_Y: return EKey::Key_Y;
	case GLFW_KEY_Z: return EKey::Key_Z;
	case GLFW_KEY_LEFT_BRACKET: return EKey::Key_Left_Bracket;
	case GLFW_KEY_BACKSLASH: return EKey::Key_Backslash;
	case GLFW_KEY_RIGHT_BRACKET: return EKey::Key_Right_Bracket;
	case GLFW_KEY_GRAVE_ACCENT: return EKey::Key_Grave_Accent;
	case GLFW_KEY_WORLD_1: return EKey::Key_World_1;
	case GLFW_KEY_WORLD_2: return EKey::Key_World_2;
	case GLFW_KEY_ESCAPE: return EKey::Key_Escape;
	case GLFW_KEY_ENTER: return EKey::Key_Enter;
	case GLFW_KEY_TAB: return EKey::Key_Tab;
	case GLFW_KEY_BACKSPACE: return EKey::Key_Backspace;
	case GLFW_KEY_INSERT: return EKey::Key_Insert;
	case GLFW_KEY_DELETE: return EKey::Key_Delete;
	case GLFW_KEY_RIGHT: return EKey::Key_Right;
	case GLFW_KEY_LEFT: return EKey::Key_Left;
	case GLFW_KEY_DOWN: return EKey::Key_Down;
	case GLFW_KEY_UP: return EKey::Key_Up;
	case GLFW_KEY_PAGE_UP: return EKey::Key_Page_Up;
	case GLFW_KEY_PAGE_DOWN: return EKey::Key_Page_Down;
	case GLFW_KEY_HOME: return EKey::Key_Home;
	case GLFW_KEY_END: return EKey::Key_End;
	case GLFW_KEY_CAPS_LOCK: return EKey::Key_Caps_Lock;
	case GLFW_KEY_SCROLL_LOCK: return EKey::Key_Scroll_Lock;
	case GLFW_KEY_NUM_LOCK: return EKey::Key_Num_Lock;
	case GLFW_KEY_PRINT_SCREEN: return EKey::Key_Print_Screen;
	case GLFW_KEY_PAUSE: return EKey::Key_Pause;
	case GLFW_KEY_F1: return EKey::Key_F1;
	case GLFW_KEY_F2: return EKey::Key_F2;
	case GLFW_KEY_F3: return EKey::Key_F3;
	case GLFW_KEY_F4: return EKey::Key_F4;
	case GLFW_KEY_F5: return EKey::Key_F5;
	case GLFW_KEY_F6: return EKey::Key_F6;
	case GLFW_KEY_F7: return EKey::Key_F7;
	case GLFW_KEY_F8: return EKey::Key_F8;
	case GLFW_KEY_F9: return EKey::Key_F9;
	case GLFW_KEY_F10: return EKey::Key_F10;
	case GLFW_KEY_F11: return EKey::Key_F11;
	case GLFW_KEY_F12: return EKey::Key_F12;
	case GLFW_KEY_F13: return EKey::Key_F13;
	case GLFW_KEY_F14: return EKey::Key_F14;
	case GLFW_KEY_F15: return EKey::Key_F15;
	case GLFW_KEY_F16: return EKey::Key_F16;
	case GLFW_KEY_F17: return EKey::Key_F17;
	case GLFW_KEY_F18: return EKey::Key_F18;
	case GLFW_KEY_F19: return EKey::Key_F19;
	case GLFW_KEY_F20: return EKey::Key_F20;
	case GLFW_KEY_F21: return EKey::Key_F21;
	case GLFW_KEY_F22: return EKey::Key_F22;
	case GLFW_KEY_F23: return EKey::Key_F23;
	case GLFW_KEY_F24: return EKey::Key_F24;
	case GLFW_KEY_F25: return EKey::Key_F25;
	case GLFW_KEY_KP_0: return EKey::Key_Numpad_0;
	case GLFW_KEY_KP_1: return EKey::Key_Numpad_1;
	case GLFW_KEY_KP_2: return EKey::Key_Numpad_2;
	case GLFW_KEY_KP_3: return EKey::Key_Numpad_3;
	case GLFW_KEY_KP_4: return EKey::Key_Numpad_4;
	case GLFW_KEY_KP_5: return EKey::Key_Numpad_5;
	case GLFW_KEY_KP_6: return EKey::Key_Numpad_6;
	case GLFW_KEY_KP_7: return EKey::Key_Numpad_7;
	case GLFW_KEY_KP_8: return EKey::Key_Numpad_8;
	case GLFW_KEY_KP_9: return EKey::Key_Numpad_9;
	case GLFW_KEY_KP_DECIMAL: return EKey::Key_Numpad_Decimal;
	case GLFW_KEY_KP_DIVIDE: return EKey::Key_Numpad_Divide;
	case GLFW_KEY_KP_MULTIPLY: return EKey::Key_Numpad_Multiply;
	case GLFW_KEY_KP_SUBTRACT: return EKey::Key_Numpad_Subtract;
	case GLFW_KEY_KP_ADD: return EKey::Key_Numpad_Add;
	case GLFW_KEY_KP_ENTER: return EKey::Key_Numpad_Enter;
	case GLFW_KEY_KP_EQUAL: return EKey::Key_Numpad_Equal;
	case GLFW_KEY_LEFT_SHIFT: return EKey::Key_Left_Shift;
	case GLFW_KEY_LEFT_CONTROL: return EKey::Key_Left_Control;
	case GLFW_KEY_LEFT_ALT: return EKey::Key_Left_Alt;
	case GLFW_KEY_LEFT_SUPER: return EKey::Key_Left_Super;
	case GLFW_KEY_RIGHT_SHIFT: return EKey::Key_Right_Shift;
	case GLFW_KEY_RIGHT_CONTROL: return EKey::Key_Right_Control;
	case GLFW_KEY_RIGHT_ALT: return EKey::Key_Right_Alt;
	case GLFW_KEY_RIGHT_SUPER: return EKey::Key_Right_Super;
	case GLFW_KEY_MENU: return EKey::Key_Menu;
	}

	return EKey::Key_Unknown;
}

static EMouseButton GLFWMouseButtonToEMouseButton(int16_t glfwKey)
{
	switch (glfwKey)
	{
	case GLFW_MOUSE_BUTTON_1: return EMouseButton::Mouse_Left;
	case GLFW_MOUSE_BUTTON_2: return EMouseButton::Mouse_Right;
	case GLFW_MOUSE_BUTTON_3: return EMouseButton::Mouse_Center;
	case GLFW_MOUSE_BUTTON_4: return EMouseButton::Mouse_Button4;
	case GLFW_MOUSE_BUTTON_5: return EMouseButton::Mouse_Button5;
	case GLFW_MOUSE_BUTTON_6: return EMouseButton::Mouse_Button6;
	case GLFW_MOUSE_BUTTON_7: return EMouseButton::Mouse_Button7;
	case GLFW_MOUSE_BUTTON_8: return EMouseButton::Mouse_Button8;
	}

	return EMouseButton::Mouse_Unknown;
}

static EKeyState WindowEventTypeToEKeyState(EWindowEventType windowEventType)
{
	switch (windowEventType)
	{
	case EWindowEventType::KeyPressed: return EKeyState::Pressed;
	case EWindowEventType::KeyReleased: return EKeyState::Released;
	case EWindowEventType::KeyRepeated: return EKeyState::Repeated;
	case EWindowEventType::MousePressed: return EKeyState::Pressed;
	case EWindowEventType::MouseReleased: return EKeyState::Released;
	default: return EKeyState::Unknown;
	}
}
#pragma endregion

void InputSubsystem::ProcessQueue()
{
	mouseScroll = glm::vec2{};
	cursorPreviousPosition = cursorPosition;

	for (const std::shared_ptr<IWindowEvent>& windowEvent : queueEvents)
	{
		switch (windowEvent->GetEventType())
		{
		case EWindowEventType::KeyPressed:
		case EWindowEventType::KeyReleased:
		case EWindowEventType::KeyRepeated:	// TODO: may need to handle repeat manually https://www.glfw.org/docs/3.3/input_guide.html#input_keyboard
		{
			auto keyEvent = std::static_pointer_cast<KeyButtonEvent>(windowEvent);
			EKey key = GLFWKeyToEKey(keyEvent->GetButton());
			EKeyState keyState = WindowEventTypeToEKeyState(windowEvent->GetEventType());
			keyStates[(size_t)key] = keyState;

			inputKeyDelegate.Broadcast(key, keyState);
		}
		break;

		case EWindowEventType::MousePressed:
		case EWindowEventType::MouseReleased:
		{
			auto keyEvent = std::static_pointer_cast<MouseButtonEvent>(windowEvent);
			EMouseButton button = GLFWMouseButtonToEMouseButton(keyEvent->GetButton());
			EKeyState mouseState = WindowEventTypeToEKeyState(windowEvent->GetEventType());
			mouseStates[(size_t)button] = mouseState;

			inputMouseDelegate.Broadcast(button, mouseState);
		}
		break;
		case EWindowEventType::MouseScrolled:
		{
			auto mouseEvent = std::static_pointer_cast<MouseScrolledEvent>(windowEvent);
			mouseScroll.x += mouseEvent->GetOffsetX();
			mouseScroll.y += mouseEvent->GetOffsetY();
		}
		break;
		case EWindowEventType::CursorPosition:
		{
			auto mouseEvent = std::static_pointer_cast<CursorPositionEvent>(windowEvent);
			cursorPosition.x = mouseEvent->GetPositionX();
			cursorPosition.y = mouseEvent->GetPositionY();
		}
		break;
		case EWindowEventType::CursorEnterChanged:
		{
			auto mouseEvent = std::static_pointer_cast<CursorEnterChangedEvent>(windowEvent);
			inputCursorEnterChangedDelegate.Broadcast(mouseEvent->IsEntered());
		}
		break;
		default:
			LogError("InputSystem: Unsupported event %s", windowEvent->GetName());
			break;
		}
	}
	queueEvents.clear();

	cursorDelta = cursorPosition - cursorPreviousPosition;
	if (cursorDelta.x != 0 || cursorDelta.y != 0)
	{
		inputCursorPositionDelegate.Broadcast(cursorPosition);
		inputCursorMoveDelegate.Broadcast(cursorDelta);
	}

	if (mouseScroll.x != 0 || mouseScroll.y != 0)
	{
		inputMouseScrollDelegate.Broadcast(mouseScroll);
	}
}

void InputSubsystem::AddQueue(const std::shared_ptr<IWindowEvent>& windowEvent)
{
	queueEvents.push_back(windowEvent);
}

bool InputSubsystem::IsKeyPressed(EKey key) const
{
	return keyStates[(size_t)key] == EKeyState::Pressed;
}

bool InputSubsystem::IsKeyReleased(EKey key) const
{
	return keyStates[(size_t)key] == EKeyState::Released;
}

bool InputSubsystem::IsKeyRepeated(EKey key) const
{
	return keyStates[(size_t)key] == EKeyState::Repeated;
}

bool InputSubsystem::IsMouseButtonPressed(EMouseButton button) const
{
	return mouseStates[(size_t)button] == EKeyState::Pressed;
}

bool InputSubsystem::IsMouseButtonReleased(EMouseButton button) const
{
	return mouseStates[(size_t)button] == EKeyState::Released;
}

glm::dvec2 InputSubsystem::GetMouseScroll() const
{
	return mouseScroll;
}

glm::dvec2 InputSubsystem::GetCursorPosition() const
{
	return cursorPosition;
}

glm::dvec2 InputSubsystem::GetCursorDelta() const
{
	return cursorDelta;
}
}