#include "pch.h"
#include "renderer/Model.h"
#include "core/Logger.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

namespace FGEngine
{
	Model::Model(const std::string& file)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(file, 0);

		Check(scene, "Unable to load model from %s", file.c_str());
	}
}
