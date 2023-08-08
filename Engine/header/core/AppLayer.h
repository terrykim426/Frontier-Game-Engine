#pragma once

#include "core/Core.h"

namespace FGEngine
{
	class ENGINE_API AppLayer
	{
	public:
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
	};
}