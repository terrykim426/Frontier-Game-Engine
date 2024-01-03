#pragma once

#include <string>

namespace FGEngine
{
	class Shader
	{
	public:
		enum class EType : uint8_t
		{
			Vertex,
			Fragment,
		};

	public:
		Shader(const std::string& filename, EType inShaderType);

		std::string GetShaderFilename() const { return shaderFilename; }
		std::vector<char> GetShaderCode() const { return shaderCode; }
		EType GetType() const { return shaderType; }

	private:
		std::string shaderFilename;
		std::vector<char> shaderCode;
		EType shaderType;
	};
}

