#pragma once

#include "renderer/Mesh.h"
#include "renderer/Texture.h"

#include <string>
#include <vector>

namespace FGEngine
{
	class Model
	{
	public:
		static Model* GenerateQuad();

	private:
		Model() = default;

	public:
		Model(const std::string& file);

		size_t GetMeshCount() const { return meshes.size(); }
		const Mesh* GetMesh(uint32_t index = 0) const;

		void SetTexture(const Texture& texture, uint32_t index = 0);
		void SetTexture(Texture&& texture, uint32_t index = 0);
		const Texture* GetTexture(uint32_t index = 0) const;

	private:
		std::vector<Mesh> meshes;
		std::vector<Texture> textures;
	};
}