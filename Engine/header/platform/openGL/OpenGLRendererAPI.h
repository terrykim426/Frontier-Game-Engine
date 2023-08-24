#pragma once

#include "renderer/RendererAPI.h"

namespace FGEngine
{
	class OpenGLRendererAPI : public IRendererAPI
	{
	public:
		// Inherited via IRendererAPI
		virtual void SetClearColor(float r, float g, float b, float a) override;
		virtual void Clear() const override;
		virtual void Render(void* nativeWindow) override;
		virtual void Resize() override;

		virtual std::string GetName() const override;
		virtual std::string GetVersion() const override;

	public:
		static bool IsSupported();
	};
}