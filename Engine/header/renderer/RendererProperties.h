#pragma once

struct GLFWwindow;

namespace FGEngine
{
	enum class ERendererAPI : char
	{
		None,
		OpenGL,
		Vulkan,
	};

	struct RendererProperties
	{
	public:
		RendererProperties() = default;
		GLFWwindow* nativeWindow;

		RendererProperties(ERendererAPI rendererAPI, GLFWwindow* window) :
			rendererAPI(rendererAPI),
			nativeWindow(window)
		{
		}

	public:
		ERendererAPI rendererAPI;
	};
}