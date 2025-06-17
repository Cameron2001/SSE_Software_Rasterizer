#include "Mesh.h"
#include <stdexcept>
#include <cassert>

Mesh::Mesh(VertexArray vertexArray) : mVertexArray(std::move(vertexArray)), mLocalMatrix(1.0f)
{
	if (mVertexArray.size() == 0)
	{
		throw std::invalid_argument("Vertex array cannot be empty");
	}

	validateVertexArray();
}

Mesh::Mesh(VertexArray vertexArray, const std::shared_ptr<Material>& material) : mVertexArray(std::move(vertexArray)),
	mLocalMatrix(1.0f), mMaterial(material)
{
	if (mVertexArray.size() == 0)
	{
		throw std::invalid_argument("Vertex array cannot be empty");
	}

	if (!material)
	{
		throw std::invalid_argument("Material cannot be null");
	}

	validateVertexArray();
}

void Mesh::validateVertexArray() const
{
	if (mVertexArray.positionsX.size() != mVertexArray.positionsY.size() ||
		mVertexArray.positionsY.size() != mVertexArray.positionsZ.size())
	{
		throw std::invalid_argument("Position arrays must have the same size");
	}

	if (mVertexArray.normalsX.size() != mVertexArray.normalsY.size() ||
		mVertexArray.normalsY.size() != mVertexArray.normalsZ.size())
	{
		throw std::invalid_argument("Normal arrays must have the same size");
	}

	if (mVertexArray.uvsU.size() != mVertexArray.uvsV.size())
	{
		throw std::invalid_argument("UV arrays must have the same size");
	}
}

void Mesh::setLocalMatrix(const glm::mat4& matrix)
{
	mLocalMatrix = matrix;
}

void Mesh::setMaterial(const std::shared_ptr<Material>& material)
{
	if (!material)
	{
		throw std::invalid_argument("Material cannot be null");
	}

	mMaterial = material;
}
