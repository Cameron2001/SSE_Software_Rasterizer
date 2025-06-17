#include <gtest/gtest.h>
#include "../src/VertexArray.h"

class VertexArrayTest : public testing::Test
{
protected:
	void SetUp() override
	{
		vertexArray = std::make_unique<VertexArray>();
	}

	std::unique_ptr<VertexArray> vertexArray;
};

TEST_F(VertexArrayTest, DefaultConstruction)
{
	EXPECT_EQ(vertexArray->size(), 0);
	EXPECT_TRUE(vertexArray->positionsX.empty());
	EXPECT_TRUE(vertexArray->positionsY.empty());
	EXPECT_TRUE(vertexArray->positionsZ.empty());
	EXPECT_TRUE(vertexArray->uvsU.empty());
	EXPECT_TRUE(vertexArray->uvsV.empty());
	EXPECT_TRUE(vertexArray->normalsX.empty());
	EXPECT_TRUE(vertexArray->normalsY.empty());
	EXPECT_TRUE(vertexArray->normalsZ.empty());
}

TEST_F(VertexArrayTest, Resize)
{
	constexpr size_t newSize = 100;
	vertexArray->resize(newSize);

	EXPECT_EQ(vertexArray->size(), newSize);
	EXPECT_EQ(vertexArray->positionsX.size(), newSize);
	EXPECT_EQ(vertexArray->positionsY.size(), newSize);
	EXPECT_EQ(vertexArray->positionsZ.size(), newSize);
	EXPECT_EQ(vertexArray->uvsU.size(), newSize);
	EXPECT_EQ(vertexArray->uvsV.size(), newSize);
	EXPECT_EQ(vertexArray->normalsX.size(), newSize);
	EXPECT_EQ(vertexArray->normalsY.size(), newSize);
	EXPECT_EQ(vertexArray->normalsZ.size(), newSize);
}

TEST_F(VertexArrayTest, Reserve)
{
	constexpr size_t reserveSize = 1000;
	vertexArray->reserve(reserveSize);
	EXPECT_EQ(vertexArray->size(), 0);
	EXPECT_GE(vertexArray->positionsX.capacity(), reserveSize);
	EXPECT_GE(vertexArray->positionsY.capacity(), reserveSize);
	EXPECT_GE(vertexArray->positionsZ.capacity(), reserveSize);
	EXPECT_GE(vertexArray->uvsU.capacity(), reserveSize);
	EXPECT_GE(vertexArray->uvsV.capacity(), reserveSize);
	EXPECT_GE(vertexArray->normalsX.capacity(), reserveSize);
	EXPECT_GE(vertexArray->normalsY.capacity(), reserveSize);
	EXPECT_GE(vertexArray->normalsZ.capacity(), reserveSize);
}

TEST_F(VertexArrayTest, Clear)
{
	vertexArray->resize(50);
	EXPECT_EQ(vertexArray->size(), 50);

	vertexArray->clear();
	EXPECT_EQ(vertexArray->size(), 0);
	EXPECT_TRUE(vertexArray->positionsX.empty());
	EXPECT_TRUE(vertexArray->positionsY.empty());
	EXPECT_TRUE(vertexArray->positionsZ.empty());
	EXPECT_TRUE(vertexArray->uvsU.empty());
	EXPECT_TRUE(vertexArray->uvsV.empty());
	EXPECT_TRUE(vertexArray->normalsX.empty());
	EXPECT_TRUE(vertexArray->normalsY.empty());
	EXPECT_TRUE(vertexArray->normalsZ.empty());
}

TEST_F(VertexArrayTest, DataIntegrity)
{
	constexpr size_t testSize = 3;
	vertexArray->resize(testSize);

	vertexArray->positionsX = {1.0f, 2.0f, 3.0f};
	vertexArray->positionsY = {4.0f, 5.0f, 6.0f};
	vertexArray->positionsZ = {7.0f, 8.0f, 9.0f};

	vertexArray->uvsU = {0.0f, 0.5f, 1.0f};
	vertexArray->uvsV = {0.0f, 0.5f, 1.0f};

	vertexArray->normalsX = {1.0f, 0.0f, 0.0f};
	vertexArray->normalsY = {0.0f, 1.0f, 0.0f};
	vertexArray->normalsZ = {0.0f, 0.0f, 1.0f};

	EXPECT_FLOAT_EQ(vertexArray->positionsX[0], 1.0f);
	EXPECT_FLOAT_EQ(vertexArray->positionsY[1], 5.0f);
	EXPECT_FLOAT_EQ(vertexArray->positionsZ[2], 9.0f);

	EXPECT_FLOAT_EQ(vertexArray->uvsU[1], 0.5f);
	EXPECT_FLOAT_EQ(vertexArray->uvsV[2], 1.0f);

	EXPECT_FLOAT_EQ(vertexArray->normalsX[0], 1.0f);
	EXPECT_FLOAT_EQ(vertexArray->normalsY[1], 1.0f);
	EXPECT_FLOAT_EQ(vertexArray->normalsZ[2], 1.0f);
}

TEST_F(VertexArrayTest, SizeConsistency)
{
	constexpr size_t testSize = 100;
	vertexArray->resize(testSize);

	EXPECT_EQ(vertexArray->positionsX.size(), testSize);
	EXPECT_EQ(vertexArray->positionsY.size(), testSize);
	EXPECT_EQ(vertexArray->positionsZ.size(), testSize);
	EXPECT_EQ(vertexArray->uvsU.size(), testSize);
	EXPECT_EQ(vertexArray->uvsV.size(), testSize);
	EXPECT_EQ(vertexArray->normalsX.size(), testSize);
	EXPECT_EQ(vertexArray->normalsY.size(), testSize);
	EXPECT_EQ(vertexArray->normalsZ.size(), testSize);
}

TEST_F(VertexArrayTest, RepeatedOperations)
{
	for (size_t i = 1; i <= 10; ++i)
	{
		vertexArray->resize(i * 10);
		EXPECT_EQ(vertexArray->size(), i * 10);
	}
	for (int i = 0; i < 5; ++i)
	{
		vertexArray->resize(50);
		EXPECT_EQ(vertexArray->size(), 50);

		vertexArray->clear();
		EXPECT_EQ(vertexArray->size(), 0);
	}
}

TEST_F(VertexArrayTest, LargeSize)
{
	constexpr size_t largeSize = 100000;

	EXPECT_NO_THROW(vertexArray->resize(largeSize));
	EXPECT_EQ(vertexArray->size(), largeSize);

	EXPECT_NO_THROW(vertexArray->clear());
	EXPECT_EQ(vertexArray->size(), 0);
}

TEST_F(VertexArrayTest, ZeroSize)
{
	vertexArray->resize(100);
	EXPECT_EQ(vertexArray->size(), 100);

	EXPECT_THROW(vertexArray->resize(0), std::invalid_argument);
}

TEST_F(VertexArrayTest, ReserveVsResize)
{
	constexpr size_t testSize = 1000;

	vertexArray->reserve(testSize);
	EXPECT_EQ(vertexArray->size(), 0);
	EXPECT_GE(vertexArray->positionsX.capacity(), testSize);

	vertexArray->resize(testSize);
	EXPECT_EQ(vertexArray->size(), testSize);
}

TEST_F(VertexArrayTest, InitializationValues)
{
	constexpr size_t testSize = 5;
	vertexArray->resize(testSize);
	for (size_t i = 0; i < testSize; ++i)
	{
		EXPECT_FLOAT_EQ(vertexArray->positionsX[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->positionsY[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->positionsZ[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->uvsU[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->uvsV[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->normalsX[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->normalsY[i], 0.0f);
		EXPECT_FLOAT_EQ(vertexArray->normalsZ[i], 0.0f);
	}
}

TEST_F(VertexArrayTest, DataPersistenceAfterResize)
{
	vertexArray->resize(3);
	vertexArray->positionsX[0] = 1.0f;
	vertexArray->positionsX[1] = 2.0f;
	vertexArray->positionsX[2] = 3.0f;
	vertexArray->resize(5);
	EXPECT_FLOAT_EQ(vertexArray->positionsX[0], 1.0f);
	EXPECT_FLOAT_EQ(vertexArray->positionsX[1], 2.0f);
	EXPECT_FLOAT_EQ(vertexArray->positionsX[2], 3.0f);
	EXPECT_FLOAT_EQ(vertexArray->positionsX[3], 0.0f);
	EXPECT_FLOAT_EQ(vertexArray->positionsX[4], 0.0f);
}

TEST_F(VertexArrayTest, MemoryEfficiency)
{
	vertexArray->reserve(1000);
	for (size_t i = 1; i <= 100; ++i)
	{
		vertexArray->resize(i);
		EXPECT_EQ(vertexArray->size(), i);
	}
}

TEST_F(VertexArrayTest, ResizeValidation)
{
	EXPECT_THROW(vertexArray->resize(0), std::invalid_argument);

	EXPECT_NO_THROW(vertexArray->resize(1));
	EXPECT_NO_THROW(vertexArray->resize(100));
	EXPECT_NO_THROW(vertexArray->resize(10000));
}

TEST_F(VertexArrayTest, ReserveValidation)
{
	EXPECT_THROW(vertexArray->reserve(0), std::invalid_argument);

	EXPECT_NO_THROW(vertexArray->reserve(1));
	EXPECT_NO_THROW(vertexArray->reserve(1000));
	EXPECT_NO_THROW(vertexArray->reserve(100000));
}
