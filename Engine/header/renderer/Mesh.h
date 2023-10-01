#pragma once

#include "renderer/Vertex.h"

struct aiMesh;

namespace FGEngine
{
	class Mesh
	{
	public:
		Mesh(const aiMesh* mesh);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) :
			vertices(vertices), 
			indices(indices)
		{
		}

		const std::vector<Vertex>& GetVertices() const { return vertices; }
		const std::vector<uint32_t>& GetIndices() const { return indices; }

	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};
}
