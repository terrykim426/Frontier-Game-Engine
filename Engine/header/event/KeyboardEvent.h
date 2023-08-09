#pragma once

#include "event/WindowEvent.h"

namespace FGEngine
{

	class KeyButtonEvent : public IWindowEvent
	{
	public:
		KeyButtonEvent(int key, int mods)
			: key(key)
			, mods(mods)
		{
		}

		int GetButton() { return key; }
		int GetMods() { return mods; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << key << " - " << mods;
			return ss.str();
		}

	private:
		int key;
		int mods;
	};

	class KeyPressedEvent : public KeyButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyPressed);

		KeyPressedEvent(int key, int mods) : KeyButtonEvent(key, mods)
		{
		}
	};

	class KeyReleasedEvent : public KeyButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyReleased);

		KeyReleasedEvent(int key, int mods) : KeyButtonEvent(key, mods)
		{
		}
	};

	class KeyRepeatedEvent : public KeyButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyRepeated);

		KeyRepeatedEvent(int key, int mods) : KeyButtonEvent(key, mods)
		{
		}
	};

} // namespace FGEngine