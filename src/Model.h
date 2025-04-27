#pragma once
#include <vector>
#include <string>
#include <glm/ext/matrix_transform.hpp>

#include "Mesh.h"

class Model
{
public:
	explicit Model(const std::vector<Mesh>& meshes);

	explicit Model(const std::string& filename);

	void loadModel(const std::string& filename);

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::vec3& rotation);
	void setScale(const glm::vec3& scale);
	void setModelMatrix(const glm::mat4& matrix);

	glm::vec3 getPosition() const { return mPosition; }
	glm::vec3 getRotation() const { return mRotation; }
	glm::vec3 getScale() const { return mScale; }
	const glm::mat4& getModelMatrix() const { return mModelMatrix; }
	const std::vector<Mesh>& getMeshes() const { return mMeshes; }

private:
	void updateModelMatrix();

	std::vector<Mesh> mMeshes;
	glm::mat4 mModelMatrix;

	glm::vec3 mPosition;
	glm::vec3 mRotation;
	glm::vec3 mScale;
};
