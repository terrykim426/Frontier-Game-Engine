#include "pch.h"
#include "Renderer/Mesh.h"

#include "assimp/mesh.h"

namespace FGEngine
{
	Mesh::Mesh(const aiMesh* mesh)
	{
		vertices.resize(mesh->mNumVertices);
		for (size_t i = 0; i < vertices.size(); i++)
		{
			memcpy(&vertices[i].pos, &mesh->mVertices[i], sizeof(vertices[i].pos));
			if (mesh->HasVertexColors(0))
			{
				memcpy(&vertices[i].color, &mesh->mColors[0][i], sizeof(vertices[i].color));
			}
			if (mesh->HasTextureCoords(0))
			{
				memcpy(&vertices[i].texCoord, &mesh->mTextureCoords[0][i], sizeof(vertices[i].texCoord));
			}
		}

		if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
		{
			indices.resize(mesh->mNumFaces * 3);
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				memcpy(&indices[i * 3], mesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
			}
		}
		else
		{
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
				{
					indices.push_back(mesh->mFaces[i].mIndices[j]);
				}
			}
		}
	}
}