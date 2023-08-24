#include "pch.h"
#include "platform/openGL/OpenGLRendererAPI.h"

#include "GLFW/glfw3.h"

namespace FGEngine
{
	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void OpenGLRendererAPI::Clear() const
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	std::string OpenGLRendererAPI::GetName() const
	{
		return "OpenGL";
	}

	std::string OpenGLRendererAPI::GetVersion() const
	{
		return (const char*)glGetString(GL_VERSION);
	}

	bool OpenGLRendererAPI::IsSupported()
	{
		return glGetString(GL_VERSION) != nullptr; // NOTE: this may not work, as it is not verified!
	}

	void OpenGLRendererAPI::Render(void* nativeWindow)
	{
		glfwSwapBuffers((GLFWwindow*)nativeWindow);
	}

	void OpenGLRendererAPI::Resize()
	{
	}
}
