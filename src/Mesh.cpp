#include "Mesh.h"
#include <cassert>

Mesh::Mesh(VertexArray vertexArray) : mVertexArray(std::move(vertexArray)), mLocalMatrix(1.0f)
{
	assert(mVertexArray.size()>0 && "Vertex array cannot be empty");

	validateVertexArray();
}

Mesh::Mesh(VertexArray vertexArray, const std::shared_ptr<Material>& material) : mVertexArray(std::move(vertexArray)),
	mLocalMatrix(1.0f), mMaterial(material)
{
	assert(mVertexArray.size()>0 && "Vertex array cannot be empty");
	assert(material && "Material cannot be null");

	validateVertexArray();
}

void Mesh::validateVertexArray() const
{
	assert(mVertexArray.positionsX.size() == mVertexArray.positionsY.size() &&
		mVertexArray.positionsY.size() == mVertexArray.positionsZ.size() &&
		"Position arrays must have the same size");

	assert(mVertexArray.normalsX.size() == mVertexArray.normalsY.size() &&
		mVertexArray.normalsY.size() == mVertexArray.normalsZ.size() &&
		"Normal arrays must have the same size");

	assert(mVertexArray.uvsU.size() == mVertexArray.uvsV.size() &&
		"UV arrays must have the same size");
}

void Mesh::setLocalMatrix(const glm::mat4& matrix)
{
	mLocalMatrix = matrix;
}

void Mesh::setMaterial(const std::shared_ptr<Material>& material)
{
	assert(material && "Material cannot be null");
	mMaterial = material;
}
