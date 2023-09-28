#include "pch.h"
#include "renderer/Texture.h"
#include "core/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace FGEngine
{
	Texture::Texture(const std::string& path)
	{
		texturePtr = stbi_load(path.c_str(), &width, &height, &channelCount, STBI_rgb_alpha);
		Check(texturePtr, "failed to load texture from %s", path.c_str());
	}

	Texture::~Texture()
	{
		stbi_image_free(texturePtr);
	}
}