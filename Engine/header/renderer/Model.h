#pragma once

#include "renderer/Mesh.h"

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
		const Mesh* GetMesh(int index) const;

	private:
		std::vector<Mesh> meshes;
	};
}