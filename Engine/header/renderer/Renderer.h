#pragma once

#include <memory>

#include "renderer/RendererProperties.h"

namespace FGEngine
{
	class Renderer
	{
	public:
		static void Init(const RendererProperties& rendererProperties = RendererProperties());
		static void Shutdown();

		static void SetClearColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
		static void Clear();

		static void Render(void* nativeWindow);

		static void Resize();

	private:
		static std::unique_ptr<class IRendererAPI> s_api;
	};
}