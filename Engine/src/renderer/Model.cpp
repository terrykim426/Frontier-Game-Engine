#include "pch.h"
#include "renderer/Model.h"
#include "core/Logger.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace FGEngine
{
	Model* Model::GenerateQuad()
	{
		const std::vector<Vertex> vertices =
		{
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		Model* model = new Model();
		model->meshes.emplace_back(vertices, indices);
		return model;
	}

	Model::Model(const std::string& file)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(file, 
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType
		);

		Check(scene, "Unable to load model from %s", file.c_str());

		meshes.reserve(scene->mNumMeshes);
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			meshes.emplace_back(scene->mMeshes[i]);
		}
	}

	const Mesh* Model::GetMesh(uint32_t index) const
	{
		if (index >= meshes.size()) return nullptr;
		return &meshes[index];
	}

	void Model::SetTexture(const Texture& texture, uint32_t index)
	{
		if (textures.size() <= index)
		{
			textures.resize(index + 1);
		}

		textures[index] = texture;
	}

	void Model::SetTexture(Texture&& texture, uint32_t index)
	{
		if (textures.size() <= index)
		{
			textures.resize(index + 1);
		}

		textures[index] = std::move(texture);
	}

	const Texture* Model::GetTexture(uint32_t index) const
	{
		if (index >= textures.size()) return nullptr;
		return &textures[index];
	}
}
