#pragma once

#include <string>

namespace FGEngine
{
	class Texture
	{
	public:
		Texture() = default;
		Texture(const std::string& path);
		Texture(const Texture& other);
		Texture(Texture&& other) noexcept;
		~Texture();

		Texture& operator=(const Texture& other);
		Texture& operator=(Texture&& other) noexcept;

		int GetWidth() const { return width; }
		int GetHeight() const { return height; }
		int GetChannelCount() const { return channelCount; }
		unsigned char* const Data() const { return texturePtr; }


	private:
		int width;
		int height;
		int channelCount;
		unsigned char* texturePtr;
	};
}

