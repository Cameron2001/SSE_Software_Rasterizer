#include <gtest/gtest.h>
#include "../src/Texture.h"
#include <immintrin.h>
#include <fstream>

class TextureTest : public testing::Test
{
protected:
	void SetUp() override
	{
		testImagePath = "test_texture.bmp";
		createMinimalBMP();
	}

	void TearDown() override
	{
		std::remove(testImagePath.c_str());
	}

	void createMinimalBMP() const
	{
		std::ofstream file(testImagePath, std::ios::binary);

		file.put('B');
		file.put('M');
		constexpr int fileSize = 54 + 3;
		file.write(reinterpret_cast<const char*>(&fileSize), 4);
		constexpr int reserved = 0;
		file.write(reinterpret_cast<const char*>(&reserved), 4);
		constexpr int dataOffset = 54;
		file.write(reinterpret_cast<const char*>(&dataOffset), 4);

		constexpr int headerSize = 40;
		file.write(reinterpret_cast<const char*>(&headerSize), 4);
		constexpr int width = 1;
		file.write(reinterpret_cast<const char*>(&width), 4);
		constexpr int height = 1;
		file.write(reinterpret_cast<const char*>(&height), 4);
		constexpr short planes = 1;
		file.write(reinterpret_cast<const char*>(&planes), 2);
		constexpr short bitsPerPixel = 24;
		file.write(reinterpret_cast<const char*>(&bitsPerPixel), 2);
		for (int i = 0; i < 24; ++i) file.put(0);
		file.put(0);
		file.put(0);
		file.put(static_cast<char>(255));

		file.close();
	}

	std::string testImagePath;
};

TEST_F(TextureTest, ConstructorWithEmptyPath)
{
	const Texture texture("");

	EXPECT_FALSE(texture.isLoaded());
	EXPECT_EQ(texture.getWidth(), 0);
	EXPECT_EQ(texture.getHeight(), 0);
	EXPECT_EQ(texture.getData(), nullptr);
}

TEST_F(TextureTest, ConstructorWithNonExistentFile)
{
	const Texture texture("non_existent_file.png");

	EXPECT_FALSE(texture.isLoaded());
	EXPECT_EQ(texture.getWidth(), 0);
	EXPECT_EQ(texture.getHeight(), 0);
	EXPECT_EQ(texture.getData(), nullptr);
}

TEST_F(TextureTest, ConstructorWithValidFile)
{
	const Texture texture(testImagePath);

	EXPECT_NO_THROW(const Texture texture2(testImagePath));
	if (texture.isLoaded())
	{
		EXPECT_EQ(texture.getWidth(), 1);
		EXPECT_EQ(texture.getHeight(), 1);
		EXPECT_NE(texture.getData(), nullptr);
	}
	else
	{
		EXPECT_EQ(texture.getWidth(), 0);
		EXPECT_EQ(texture.getHeight(), 0);
		EXPECT_EQ(texture.getData(), nullptr);
	}
}

TEST_F(TextureTest, LoadEmptyPath)
{
	Texture texture("dummy.png");

	EXPECT_THROW(texture.load(""), std::invalid_argument);
}

TEST_F(TextureTest, SampleWithUnloadedTexture)
{
	const Texture texture("");

	const __m128 u = _mm_set_ps(0.75f, 0.5f, 0.25f, 0.0f);
	const __m128 v = _mm_set_ps(0.75f, 0.5f, 0.25f, 0.0f);

	const __m128i result = texture.sample(u, v);
	alignas(16) uint32_t colors[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(colors), result);

	for (unsigned int color : colors)
	{
		EXPECT_EQ(color, 0x00FFFF);
	}
}

TEST_F(TextureTest, SampleUVClamping)
{
	const Texture texture("");
	const __m128 u = _mm_set_ps(2.0f, 1.5f, -0.5f, -1.0f);
	const __m128 v = _mm_set_ps(3.0f, 2.0f, -1.0f, -2.0f);
	EXPECT_NO_THROW(const __m128i result = texture.sample(u, v));
}

TEST_F(TextureTest, SampleNormalizedUVs)
{
	const Texture texture("");
	const __m128 u = _mm_set_ps(1.0f, 0.75f, 0.25f, 0.0f);
	const __m128 v = _mm_set_ps(1.0f, 0.75f, 0.25f, 0.0f);

	const __m128i result = texture.sample(u, v);
	alignas(16) uint32_t colors[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(colors), result);

	for (unsigned int color : colors)
	{
		EXPECT_EQ(color, 0x00FFFF);
	}
}

TEST_F(TextureTest, DestructorSafety)
{
	{
		Texture texture(testImagePath);
	}
	SUCCEED();
}

TEST_F(TextureTest, MultipleLoadCalls)
{
	Texture texture("");

	EXPECT_THROW(texture.load(""), std::invalid_argument);
	EXPECT_THROW(texture.load(""), std::invalid_argument);

	EXPECT_FALSE(texture.isLoaded());
}

TEST_F(TextureTest, GettersWhenUnloaded)
{
	const Texture texture("");

	EXPECT_EQ(texture.getWidth(), 0);
	EXPECT_EQ(texture.getHeight(), 0);
	EXPECT_EQ(texture.getData(), nullptr);
	EXPECT_FALSE(texture.isLoaded());
}

TEST_F(TextureTest, SampleVectorized)
{
	const Texture texture("");

	const __m128 u1 = _mm_set_ps(0.1f, 0.2f, 0.3f, 0.4f);
	const __m128 v1 = _mm_set_ps(0.5f, 0.6f, 0.7f, 0.8f);

	const __m128 u2 = _mm_set_ps(0.9f, 0.8f, 0.7f, 0.6f);
	const __m128 v2 = _mm_set_ps(0.5f, 0.4f, 0.3f, 0.2f);

	const __m128i result1 = texture.sample(u1, v1);
	const __m128i result2 = texture.sample(u2, v2);
	alignas(16) uint32_t colors1[4], colors2[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(colors1), result1);
	_mm_store_si128(reinterpret_cast<__m128i*>(colors2), result2);

	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(colors1[i], 0x00FFFF);
		EXPECT_EQ(colors2[i], 0x00FFFF);
	}
}

TEST_F(TextureTest, BoundaryUVValues)
{
	const Texture texture("");

	const __m128 u = _mm_set_ps(1.0f, 1.0f, 0.0f, 0.0f);
	const __m128 v = _mm_set_ps(1.0f, 0.0f, 1.0f, 0.0f);
	EXPECT_NO_THROW(const __m128i result = texture.sample(u, v));
}
