#include "pch.h"
#include "renderer/Texture.h"
#include "core/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace FGEngine
{
	Texture::Texture(const std::string& path)
	{
		stbi_set_flip_vertically_on_load(true);
		texturePtr = stbi_load(path.c_str(), &width, &height, &channelCount, STBI_rgb_alpha);
		Check(texturePtr, "failed to load texture from %s", path.c_str());
	}

	Texture::Texture(const Texture& other) :
		width(other.width),
		height(other.height),
		channelCount(other.channelCount),
		texturePtr(other.texturePtr)
	{
	}

	Texture::Texture(Texture&& other) noexcept :
		width(std::exchange(other.width, 0)),
		height(std::exchange(other.height, 0)),
		channelCount(std::exchange(other.channelCount, 0)),
		texturePtr(std::exchange(other.texturePtr, nullptr))
	{
	}

	Texture::~Texture()
	{
		if (texturePtr)
		{
			stbi_image_free(texturePtr);
		}
	}

	Texture& Texture::operator=(const Texture& other)
	{
		if (this != &other)
		{
			width = other.width;
			height = other.height;
			channelCount = other.channelCount;
			texturePtr = other.texturePtr;
		}
		return *this;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (this != &other)
		{
			width = std::exchange(other.width, 0);
			height = std::exchange(other.height, 0);
			channelCount = std::exchange(other.channelCount, 0);
			texturePtr = std::exchange(other.texturePtr, nullptr);
		}
		return *this;
	}
}