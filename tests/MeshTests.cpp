#include <gtest/gtest.h>
#include "../src/Mesh.h"
#include "../src/Material.h"
#include <glm/gtc/matrix_transform.hpp>

class MeshTest : public testing::Test
{
protected:
	void SetUp() override
	{
		vertexArray.resize(3);

		vertexArray.positionsX = {0.0f, 1.0f, 0.5f};
		vertexArray.positionsY = {0.0f, 0.0f, 1.0f};
		vertexArray.positionsZ = {0.0f, 0.0f, 0.0f};

		vertexArray.uvsU = {0.0f, 1.0f, 0.5f};
		vertexArray.uvsV = {0.0f, 0.0f, 1.0f};

		vertexArray.normalsX = {0.0f, 0.0f, 0.0f};
		vertexArray.normalsY = {0.0f, 0.0f, 0.0f};
		vertexArray.normalsZ = {1.0f, 1.0f, 1.0f};

		material = std::make_shared<Material>();
	}

	VertexArray vertexArray;
	std::shared_ptr<Material> material;
};

TEST_F(MeshTest, ConstructionWithVertexArray)
{
	const Mesh mesh(vertexArray);

	EXPECT_EQ(mesh.getVertexArray().size(), 3);
	EXPECT_EQ(mesh.getMaterial(), nullptr);

	constexpr glm::mat4 identity(1.0f);
	EXPECT_EQ(mesh.getLocalMatrix(), identity);
}

TEST_F(MeshTest, ConstructionWithVertexArrayAndMaterial)
{
	const Mesh mesh(vertexArray, material);

	EXPECT_EQ(mesh.getVertexArray().size(), 3);
	EXPECT_EQ(mesh.getMaterial(), material.get());

	constexpr glm::mat4 identity(1.0f);
	EXPECT_EQ(mesh.getLocalMatrix(), identity);
}

TEST_F(MeshTest, EmptyVertexArrayThrows)
{
	const VertexArray emptyArray;

	EXPECT_THROW(Mesh mesh(emptyArray), std::invalid_argument);
}

TEST_F(MeshTest, NullMaterialThrows)
{
	EXPECT_THROW(Mesh mesh(vertexArray, nullptr), std::invalid_argument);
}

TEST_F(MeshTest, SetLocalMatrix)
{
	Mesh mesh(vertexArray);

	const glm::mat4 transform = glm::scale(
		glm::rotate(
			glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f)),
			glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)
		),
		glm::vec3(2.0f, 2.0f, 2.0f)
	);

	mesh.setLocalMatrix(transform);

	EXPECT_EQ(mesh.getLocalMatrix(), transform);
}

TEST_F(MeshTest, SetMaterial)
{
	Mesh mesh(vertexArray);

	EXPECT_EQ(mesh.getMaterial(), nullptr);

	mesh.setMaterial(material);

	EXPECT_EQ(mesh.getMaterial(), material.get());
}

TEST_F(MeshTest, SetNullMaterialThrows)
{
	Mesh mesh(vertexArray, material);

	EXPECT_THROW(mesh.setMaterial(nullptr), std::invalid_argument);
}

TEST_F(MeshTest, MismatchedPositions)
{
	VertexArray invalidArray;
	invalidArray.positionsX = {1.0f, 2.0f};
	invalidArray.positionsY = {1.0f};
	invalidArray.positionsZ = {1.0f, 2.0f};
	invalidArray.uvsU = {0.0f, 1.0f};
	invalidArray.uvsV = {0.0f, 1.0f};
	invalidArray.normalsX = {0.0f, 0.0f};
	invalidArray.normalsY = {0.0f, 0.0f};
	invalidArray.normalsZ = {1.0f, 1.0f};

	EXPECT_THROW(Mesh mesh(invalidArray), std::invalid_argument);
}

TEST_F(MeshTest, MismatchedNormals)
{
	VertexArray invalidArray;
	invalidArray.positionsX = {1.0f, 2.0f};
	invalidArray.positionsY = {1.0f, 2.0f};
	invalidArray.positionsZ = {1.0f, 2.0f};
	invalidArray.uvsU = {0.0f, 1.0f};
	invalidArray.uvsV = {0.0f, 1.0f};
	invalidArray.normalsX = {0.0f, 0.0f};
	invalidArray.normalsY = {0.0f};
	invalidArray.normalsZ = {1.0f, 1.0f};

	EXPECT_THROW(Mesh mesh(invalidArray), std::invalid_argument);
}

TEST_F(MeshTest, MismatchedUVs)
{
	VertexArray invalidArray;
	invalidArray.positionsX = {1.0f, 2.0f};
	invalidArray.positionsY = {1.0f, 2.0f};
	invalidArray.positionsZ = {1.0f, 2.0f};
	invalidArray.uvsU = {0.0f, 1.0f};
	invalidArray.uvsV = {0.0f}; // Different size
	invalidArray.normalsX = {0.0f, 0.0f};
	invalidArray.normalsY = {0.0f, 0.0f};
	invalidArray.normalsZ = {1.0f, 1.0f};

	EXPECT_THROW(Mesh mesh(invalidArray), std::invalid_argument);
}

TEST_F(MeshTest, DataIntegrity)
{
	Mesh mesh(vertexArray);

	const auto& va = mesh.getVertexArray();

	EXPECT_FLOAT_EQ(va.positionsX[0], 0.0f);
	EXPECT_FLOAT_EQ(va.positionsX[1], 1.0f);
	EXPECT_FLOAT_EQ(va.positionsX[2], 0.5f);

	EXPECT_FLOAT_EQ(va.positionsY[0], 0.0f);
	EXPECT_FLOAT_EQ(va.positionsY[1], 0.0f);
	EXPECT_FLOAT_EQ(va.positionsY[2], 1.0f);

	EXPECT_FLOAT_EQ(va.uvsU[0], 0.0f);
	EXPECT_FLOAT_EQ(va.uvsU[1], 1.0f);
	EXPECT_FLOAT_EQ(va.uvsU[2], 0.5f);

	EXPECT_FLOAT_EQ(va.normalsZ[0], 1.0f);
	EXPECT_FLOAT_EQ(va.normalsZ[1], 1.0f);
	EXPECT_FLOAT_EQ(va.normalsZ[2], 1.0f);
}

TEST_F(MeshTest, MaterialOwnership)
{
	const auto mesh = std::make_unique<Mesh>(vertexArray, material);

	EXPECT_EQ(mesh->getMaterial(), material.get());

	const auto newMaterial = std::make_shared<Material>();
	mesh->setMaterial(newMaterial);

	EXPECT_EQ(mesh->getMaterial(), newMaterial.get());
	EXPECT_NE(mesh->getMaterial(), material.get());
}

TEST_F(MeshTest, ModificationAfterConstruction)
{
	const Mesh mesh(vertexArray);

	const auto& meshVertexArray = mesh.getVertexArray();

	EXPECT_EQ(meshVertexArray.size(), 3);

	EXPECT_EQ(vertexArray.size(), 3);
	EXPECT_FLOAT_EQ(vertexArray.positionsX[0], 0.0f);
}

TEST_F(MeshTest, LargeVertexArray)
{
	VertexArray largeArray;
	constexpr size_t largeSize = 10000;
	largeArray.resize(largeSize);

	for (size_t i = 0; i < largeSize; ++i)
	{
		largeArray.positionsX[i] = static_cast<float>(i);
		largeArray.positionsY[i] = static_cast<float>(i * 2);
		largeArray.positionsZ[i] = static_cast<float>(i * 3);
		largeArray.uvsU[i] = 0.0f;
		largeArray.uvsV[i] = 0.0f;
		largeArray.normalsX[i] = 0.0f;
		largeArray.normalsY[i] = 0.0f;
		largeArray.normalsZ[i] = 1.0f;
	}

	EXPECT_NO_THROW(Mesh mesh(largeArray));
}

TEST_F(MeshTest, ComprehensiveValidation)
{
	VertexArray largeArray;
	constexpr size_t largeSize = 5000;
	largeArray.resize(largeSize);

	for (size_t i = 0; i < largeSize; ++i)
	{
		largeArray.positionsX[i] = static_cast<float>(i % 1000); // Prevent overflow
		largeArray.positionsY[i] = static_cast<float>((i * 2) % 1000);
		largeArray.positionsZ[i] = static_cast<float>((i * 3) % 1000);
		largeArray.uvsU[i] = static_cast<float>(i % 100) / 100.0f;
		largeArray.uvsV[i] = static_cast<float>((i * 2) % 100) / 100.0f;
		largeArray.normalsX[i] = 0.0f;
		largeArray.normalsY[i] = 0.0f;
		largeArray.normalsZ[i] = 1.0f;
	}

	EXPECT_NO_THROW(Mesh largeMesh(largeArray));

	const Mesh testMesh(largeArray);
	const auto& retrievedArray = testMesh.getVertexArray();
	EXPECT_EQ(retrievedArray.size(), largeSize);
	EXPECT_FLOAT_EQ(retrievedArray.positionsX[100], 100.0f);
	EXPECT_FLOAT_EQ(retrievedArray.uvsU[200], 0.0f);
}
