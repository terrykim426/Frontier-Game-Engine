#include "pch.h"
#include "renderer/Shader.h"

#include <fstream>

namespace FGEngine
{
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);	// read from the end, to determine the file size

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0); // return to beginning
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}


	Shader::Shader(const std::string& filename, EType inShaderType)
	{
		shaderFilename = filename;
		shaderCode = ReadFile(filename);
		shaderType = inShaderType;
	}

}