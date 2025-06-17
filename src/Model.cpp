#include "Model.h"
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <cassert>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <glm/ext/matrix_transform.hpp>

Model::Model(const std::vector<Mesh>& meshes)
	: mMeshes(meshes), mModelMatrix(1.0f), mPosition(0.0f), mRotation(0.0f), mScale(1.0f)
{
	if (meshes.empty())
	{
		throw std::invalid_argument("Model requires at least one mesh");
	}
	updateModelMatrix();
}

Model::Model(const std::string& filename)
	: mModelMatrix(1.0f), mPosition(0.0f), mRotation(0.0f), mScale(1.0f)
{
	if (filename.empty())
	{
		throw std::invalid_argument("Model filename cannot be empty");
	}

	if (!std::filesystem::exists(filename))
	{
		throw std::runtime_error("Model file does not exist: " + filename);
	}

	loadModel(filename);
	updateModelMatrix();
}

void Model::loadModel(const std::string& filename)
{
	std::cout << "Loading model: " << filename << '\n';

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	std::filesystem::path filePath(filename);
	std::string baseDir = filePath.parent_path().string() + "/";
	bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, filename.c_str(), baseDir.c_str(), true);
	if (!warning.empty())
	{
		std::cerr << "WARN: " << warning << '\n';
	}

	if (!success)
	{
		std::string errorMsg = "Failed to load OBJ file: " + filename;
		if (!error.empty())
		{
			errorMsg += " - " + error;
		}
		throw std::runtime_error(errorMsg);
	}

	if (attrib.vertices.empty())
	{
		throw std::runtime_error("Model has no vertices: " + filename);
	}

	if (shapes.empty())
	{
		throw std::runtime_error("Model has no shapes: " + filename);
	}

	std::cout << "Model loaded: " << filename
		<< " (" << attrib.vertices.size() / 3 << " vertices, "
		<< shapes.size() << " shapes, "
		<< materials.size() << " materials)" << '\n';

	std::vector<std::shared_ptr<Material>> loadedMaterials;
	for (const auto& mat : materials)
	{
		auto material = std::make_shared<Material>();

		if (!mat.diffuse_texname.empty())
		{
			std::string texturePath = baseDir + mat.diffuse_texname;
			auto texture = std::make_shared<Texture>(texturePath);
			if (texture->isLoaded())
			{
				material->setDiffuseTexture(texture);
			}
			else
			{
				std::cerr << "Failed to load texture: " << texturePath << '\n';
			}
		}

		loadedMaterials.push_back(material);
	}

	for (size_t i = 0; i < shapes.size(); i++)
	{
		const auto& shape = shapes[i];
		VertexArray vertexArray;

		std::shared_ptr<Material> shapeMaterial = nullptr;
		if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0 &&
			shape.mesh.material_ids[0] < static_cast<int>(loadedMaterials.size()))
		{
			int matId = shape.mesh.material_ids[0];
			shapeMaterial = loadedMaterials[matId];
		}
		else if (!loadedMaterials.empty())
		{
			shapeMaterial = loadedMaterials[0]; // use first material as fallback
		}
		else
		{
			shapeMaterial = std::make_shared<Material>(); // default material
		}

		size_t indexOffset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
		{
			int fv = shape.mesh.num_face_vertices[f];

			if (fv != 3)
			{
				indexOffset += fv;
				continue;
			}

			for (size_t v = 0; v < fv; v++)
			{
				if (indexOffset + v >= shape.mesh.indices.size())
				{
					std::cerr << "Warning: Face index out of bounds in model loading\n";
					continue;
				}

				tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];

				if (idx.vertex_index < 0 || idx.vertex_index * 3 + 2 >= static_cast<int>(attrib.vertices.size()))
				{
					std::cerr << "Warning: Invalid vertex index in model loading\n";
					continue;
				}

				float posX = attrib.vertices[3 * idx.vertex_index + 0];
				float posY = attrib.vertices[3 * idx.vertex_index + 1];
				float posZ = attrib.vertices[3 * idx.vertex_index + 2];

				float uvU = 0.0f;
				float uvV = 0.0f;
				if (idx.texcoord_index >= 0 && idx.texcoord_index * 2 + 1 < static_cast<int>(attrib.texcoords.size()))
				{
					uvU = attrib.texcoords[2 * idx.texcoord_index + 0];
					uvV = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]; // flip Y 
				}

				float normalX = 0.0f;
				float normalY = 0.0f;
				float normalZ = 1.0f;
				if (idx.normal_index >= 0 && idx.normal_index * 3 + 2 < static_cast<int>(attrib.normals.size()))
				{
					normalX = attrib.normals[3 * idx.normal_index + 0];
					normalY = attrib.normals[3 * idx.normal_index + 1];
					normalZ = attrib.normals[3 * idx.normal_index + 2];
				}

				vertexArray.positionsX.push_back(posX);
				vertexArray.positionsY.push_back(posY);
				vertexArray.positionsZ.push_back(posZ);

				vertexArray.uvsU.push_back(uvU);
				vertexArray.uvsV.push_back(uvV);

				vertexArray.normalsX.push_back(normalX);
				vertexArray.normalsY.push_back(normalY);
				vertexArray.normalsZ.push_back(normalZ);
			}
			indexOffset += fv;
		}

		if (!vertexArray.positionsX.empty())
		{
			mMeshes.emplace_back(vertexArray, shapeMaterial);
		}
		else
		{
			std::cerr << "Warning: Shape " << i << " has no valid vertices and was skipped\n";
		}
	}
	if (mMeshes.empty())
	{
		throw std::runtime_error("No valid meshes were created from model: " + filename);
	}
}

void Model::setPosition(const glm::vec3& position)
{
	mPosition = position;
	updateModelMatrix();
}

void Model::setRotation(const glm::vec3& rotation)
{
	mRotation = rotation;
	updateModelMatrix();
}

void Model::setScale(const glm::vec3& scale)
{
	if (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f)
	{
		throw std::invalid_argument("Scale components must be positive");
	}
	mScale = scale;
	updateModelMatrix();
}

void Model::setModelMatrix(const glm::mat4& matrix)
{
	mModelMatrix = matrix;
}

void Model::updateModelMatrix()
{
	const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mPosition);

	const glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(mRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	const glm::mat4 rotationMatrix = rotationZ * rotationY * rotationX;

	const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), mScale);

	mModelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}
