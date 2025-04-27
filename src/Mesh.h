#pragma once
#include "VertexArray.h"
#include "Material.h"
#include "glm/mat4x4.hpp"

class Mesh
{
public:
	explicit Mesh(VertexArray vertexArray);

	Mesh(VertexArray vertexArray, const std::shared_ptr<Material>& material);

	const VertexArray& getVertexArray() const { return mVertexArray; }
	const glm::mat4& getLocalMatrix() const { return mLocalMatrix; }
	const Material* getMaterial() const { return mMaterial.get(); }

	void setLocalMatrix(const glm::mat4& matrix);
	void setMaterial(const std::shared_ptr<Material>& material);

private:
	void validateVertexArray() const;

	VertexArray mVertexArray;
	glm::mat4 mLocalMatrix = glm::mat4(1.0f);
	std::shared_ptr<Material> mMaterial;
};
