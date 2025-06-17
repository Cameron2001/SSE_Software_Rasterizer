#include <gtest/gtest.h>
#include "../src/Model.h"
#include "../src/Mesh.h"
#include <glm/gtc/matrix_transform.hpp>

class ModelTest : public testing::Test
{
protected:
	void SetUp() override
	{
		VertexArray vertexArray;
		vertexArray.resize(3);

		vertexArray.positionsX = {0.0f, 1.0f, 0.5f};
		vertexArray.positionsY = {0.0f, 0.0f, 1.0f};
		vertexArray.positionsZ = {0.0f, 0.0f, 0.0f};

		vertexArray.uvsU = {0.0f, 1.0f, 0.5f};
		vertexArray.uvsV = {0.0f, 0.0f, 1.0f};

		vertexArray.normalsX = {0.0f, 0.0f, 0.0f};
		vertexArray.normalsY = {0.0f, 0.0f, 0.0f};
		vertexArray.normalsZ = {1.0f, 1.0f, 1.0f};

		meshes = {Mesh(vertexArray)};
	}

	VertexArray vertexArray;
	std::vector<Mesh> meshes;
};

TEST_F(ModelTest, ConstructionWithMeshes)
{
	const Model model(meshes);

	EXPECT_EQ(model.getMeshes().size(), 1);
	EXPECT_EQ(model.getPosition(), glm::vec3(0.0f));
	EXPECT_EQ(model.getRotation(), glm::vec3(0.0f));
	EXPECT_EQ(model.getScale(), glm::vec3(1.0f));

	constexpr glm::mat4 identity(1.0f);
	EXPECT_EQ(model.getModelMatrix(), identity);
}

TEST_F(ModelTest, EmptyMeshVectorThrows)
{
	const std::vector<Mesh> emptyMeshes;
	EXPECT_THROW(Model model(emptyMeshes), std::invalid_argument);
}

TEST_F(ModelTest, SetPosition)
{
	Model model(meshes);

	constexpr glm::vec3 newPosition(1.0f, 2.0f, 3.0f);
	model.setPosition(newPosition);

	EXPECT_EQ(model.getPosition(), newPosition);

	constexpr glm::mat4 identity(1.0f);
	EXPECT_NE(model.getModelMatrix(), identity);
}

TEST_F(ModelTest, SetRotation)
{
	Model model(meshes);

	constexpr glm::vec3 newRotation(30.0f, 45.0f, 60.0f);
	model.setRotation(newRotation);

	EXPECT_EQ(model.getRotation(), newRotation);

	constexpr glm::mat4 identity(1.0f);
	EXPECT_NE(model.getModelMatrix(), identity);
}

TEST_F(ModelTest, SetScale)
{
	Model model(meshes);

	constexpr glm::vec3 newScale(2.0f, 3.0f, 4.0f);
	model.setScale(newScale);

	EXPECT_EQ(model.getScale(), newScale);

	constexpr glm::mat4 identity(1.0f);
	EXPECT_NE(model.getModelMatrix(), identity);
}

TEST_F(ModelTest, InvalidScaleThrows)
{
	Model model(meshes);

	EXPECT_THROW(model.setScale(glm::vec3(-1.0f, 1.0f, 1.0f)), std::invalid_argument);
	EXPECT_THROW(model.setScale(glm::vec3(1.0f, 0.0f, 1.0f)), std::invalid_argument);
	EXPECT_THROW(model.setScale(glm::vec3(1.0f, 1.0f, -0.5f)), std::invalid_argument);
}

TEST_F(ModelTest, SetModelMatrix)
{
	Model model(meshes);

	constexpr glm::mat4 customMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 6.0f, 7.0f));
	model.setModelMatrix(customMatrix);

	EXPECT_EQ(model.getModelMatrix(), customMatrix);
}

TEST_F(ModelTest, TransformationOrder)
{
	Model model(meshes);

	glm::vec3 position(1.0f, 2.0f, 3.0f);
	glm::vec3 rotation(30.0f, 45.0f, 60.0f);
	glm::vec3 scale(2.0f, 2.0f, 2.0f);

	model.setPosition(position);
	model.setRotation(rotation);
	model.setScale(scale);

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotationMatrix = rotationZ * rotationY * rotationX;
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

	glm::mat4 expectedMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	const auto& actualMatrix = model.getModelMatrix();
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(actualMatrix[i][j], expectedMatrix[i][j], 1e-5f);
		}
	}
}

TEST_F(ModelTest, ModelMatrixUpdateOnEachTransform)
{
	Model model(meshes);

	const glm::mat4 originalMatrix = model.getModelMatrix();

	model.setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::mat4 afterPosition = model.getModelMatrix();
	EXPECT_NE(afterPosition, originalMatrix);

	model.setRotation(glm::vec3(45.0f, 0.0f, 0.0f));
	const glm::mat4 afterRotation = model.getModelMatrix();
	EXPECT_NE(afterRotation, afterPosition);

	model.setScale(glm::vec3(2.0f, 2.0f, 2.0f));
	const glm::mat4 afterScale = model.getModelMatrix();
	EXPECT_NE(afterScale, afterRotation);
}

TEST_F(ModelTest, FileConstructorWithInvalidPath)
{
	EXPECT_THROW(Model model(""), std::invalid_argument);

	EXPECT_THROW(Model model("non_existent_file.obj"), std::runtime_error);
}

TEST_F(ModelTest, MultiMeshModel)
{
	VertexArray vertexArray2;
	vertexArray2.resize(3);
	vertexArray2.positionsX = {-1.0f, 0.0f, -0.5f};
	vertexArray2.positionsY = {0.0f, 0.0f, 1.0f};
	vertexArray2.positionsZ = {0.0f, 0.0f, 0.0f};
	vertexArray2.uvsU = {0.0f, 1.0f, 0.5f};
	vertexArray2.uvsV = {0.0f, 0.0f, 1.0f};
	vertexArray2.normalsX = {0.0f, 0.0f, 0.0f};
	vertexArray2.normalsY = {0.0f, 0.0f, 0.0f};
	vertexArray2.normalsZ = {1.0f, 1.0f, 1.0f};
	Mesh mesh2(vertexArray2);
	std::vector<Mesh> multiMeshes = {meshes[0], mesh2};

	Model model(multiMeshes);

	EXPECT_EQ(model.getMeshes().size(), 2);

	const auto& meshArray = model.getMeshes();
	EXPECT_EQ(meshArray[0].getVertexArray().size(), 3);
	EXPECT_EQ(meshArray[1].getVertexArray().size(), 3);
}

TEST_F(ModelTest, ZeroScale)
{
	Model model(meshes);

	EXPECT_THROW(model.setScale(glm::vec3(0.0f, 1.0f, 1.0f)), std::invalid_argument);
	EXPECT_THROW(model.setScale(glm::vec3(1.0f, 0.0f, 1.0f)), std::invalid_argument);
	EXPECT_THROW(model.setScale(glm::vec3(1.0f, 1.0f, 0.0f)), std::invalid_argument);
}

TEST_F(ModelTest, ExtremeTransformations)
{
	Model model(meshes);

	model.setPosition(glm::vec3(1000.0f, 1000.0f, 1000.0f));
	model.setRotation(glm::vec3(720.0f, 1080.0f, 1440.0f));
	model.setScale(glm::vec3(100.0f, 100.0f, 100.0f));

	const auto& matrix = model.getModelMatrix();
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			EXPECT_FALSE(std::isnan(matrix[i][j]));
			EXPECT_FALSE(std::isinf(matrix[i][j]));
		}
	}

	model.setScale(glm::vec3(0.001f, 0.001f, 0.001f));
	const auto& smallMatrix = model.getModelMatrix();
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			EXPECT_FALSE(std::isnan(smallMatrix[i][j]));
			EXPECT_FALSE(std::isinf(smallMatrix[i][j]));
		}
	}
}

TEST_F(ModelTest, NumericalStability)
{
	Model model(meshes);

	for (int i = 0; i < 100; ++i)
	{
		model.setPosition(glm::vec3(i * 0.01f, i * 0.01f, i * 0.01f));
		model.setRotation(glm::vec3(i * 0.1f, i * 0.1f, i * 0.1f));

		const glm::mat4& matrix = model.getModelMatrix();
		EXPECT_FALSE(matrix == glm::mat4(0.0f));

		bool hasNaN = false;
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				if (std::isnan(matrix[row][col]) || std::isinf(matrix[row][col]))
				{
					hasNaN = true;
					break;
				}
			}
			if (hasNaN) break;
		}
		EXPECT_FALSE(hasNaN);
	}
}
