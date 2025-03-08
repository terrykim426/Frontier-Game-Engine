#pragma once

#include <string>
#include "renderer/RendererProperties.h"

namespace FGEngine
{
	class IRendererAPI
	{
	public:
		virtual ~IRendererAPI(){}

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear() const = 0;

		virtual void Render(void* nativeWindow) = 0;
		virtual void Resize() = 0;

		virtual std::string GetName() const = 0;
		virtual std::string GetVersion() const = 0;

		static IRendererAPI* Create(const RendererProperties& rendererProperties);
	};
}