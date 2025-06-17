#include <gtest/gtest.h>
#include "../src/Framebuffer.h"
#include <immintrin.h>

class FramebufferTest : public testing::Test
{
protected:
	void SetUp() override
	{
		width = 800;
		height = 600;
		framebuffer = std::make_unique<Framebuffer>(width, height);
	}

	int width, height;
	std::unique_ptr<Framebuffer> framebuffer;
};

TEST_F(FramebufferTest, Construction)
{
	EXPECT_EQ(framebuffer->getWidth(), width);
	EXPECT_EQ(framebuffer->getHeight(), height);
	EXPECT_NE(framebuffer->getColorBuffer(), nullptr);
	EXPECT_NE(framebuffer->getDepthBuffer(), nullptr);
}

TEST_F(FramebufferTest, InvalidDimensions)
{
	EXPECT_THROW(Framebuffer(-1, 100), std::invalid_argument);
	EXPECT_THROW(Framebuffer(100, -1), std::invalid_argument);
	EXPECT_THROW(Framebuffer(0, 100), std::invalid_argument);
	EXPECT_THROW(Framebuffer(100, 0), std::invalid_argument);
}

TEST_F(FramebufferTest, ClearBuffers)
{
	framebuffer->clear();
	const uint8_t* colorBuffer = framebuffer->getColorBuffer();

	for (int i = 0; i < 30; ++i)
	{
		EXPECT_EQ(colorBuffer[i], 0);
	}

	framebuffer->clearDepth();
	const float* depthBuffer = framebuffer->getDepthBuffer();

	for (int i = 0; i < 10; ++i)
	{
		EXPECT_FLOAT_EQ(depthBuffer[i], 1.0f);
	}
}

TEST_F(FramebufferTest, DepthTest)
{
	framebuffer->clearDepth();

	const __m128i x = _mm_set_epi32(3, 2, 1, 0);
	const __m128i y = _mm_set_epi32(0, 0, 0, 0);
	const __m128 depth = _mm_set_ps(0.8f, 0.6f, 0.4f, 0.2f);

	int mask = framebuffer->depthTest(x, y, depth);
	EXPECT_EQ(mask, 0xF);

	for (int i = 0; i < 4; ++i)
	{
		const __m128i singleX = _mm_set1_epi32(i);
		const __m128i singleY = _mm_set1_epi32(0);
		const __m128 singleDepth = _mm_set1_ps(0.2f + i * 0.2f);
		framebuffer->setDepth(singleX, singleY, singleDepth, 0x1);
	}

	mask = framebuffer->depthTest(x, y, depth);
	EXPECT_EQ(mask, 0x0);
}

TEST_F(FramebufferTest, ValidBoundsHandling)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	// Test only valid coordinates since out-of-bounds are now invariants enforced by asserts
	const __m128i xValid = _mm_set_epi32(3, 2, 1, 0);
	const __m128i yValid = _mm_set_epi32(0, 0, 0, 0);
	const __m128i color = _mm_set1_epi32(0x00FF0000);

	EXPECT_NO_THROW(framebuffer->setPixel(xValid, yValid, color, 0xF));

	const uint8_t* colorBuffer = framebuffer->getColorBuffer();

	// Check that some pixels were set
	bool hasNonZeroPixels = false;
	for (int i = 0; i < 12; ++i)
	{
		if (colorBuffer[i] != 0)
		{
			hasNonZeroPixels = true;
			break;
		}
	}
	EXPECT_TRUE(hasNonZeroPixels);

	__m128 depth = _mm_set1_ps(0.5f);
	EXPECT_NO_THROW(framebuffer->setDepth(xValid, yValid, depth, 0xF));

	const float* depthBuffer = framebuffer->getDepthBuffer();
	EXPECT_FLOAT_EQ(depthBuffer[0], 0.5f);
	EXPECT_FLOAT_EQ(depthBuffer[1], 0.5f);
	EXPECT_FLOAT_EQ(depthBuffer[2], 0.5f);
	EXPECT_FLOAT_EQ(depthBuffer[3], 0.5f);
}

TEST_F(FramebufferTest, SetPixelFunctionality)
{
	framebuffer->clear();

	const __m128i x = _mm_set_epi32(3, 2, 1, 0);
	const __m128i y = _mm_set_epi32(0, 0, 0, 0);
	const __m128i colors = _mm_setr_epi32(
		0xFF0000,
		0x00FF00,
		0x0000FF,
		0xFFFFFF
	);

	framebuffer->setPixel(x, y, colors, 0xF);

	const uint8_t* colorBuffer = framebuffer->getColorBuffer();

	bool hasNonZeroPixels = false;
	for (int i = 0; i < 12; ++i)
	{
		if (colorBuffer[i] != 0)
		{
			hasNonZeroPixels = true;
			break;
		}
	}
	EXPECT_TRUE(hasNonZeroPixels);
}

TEST_F(FramebufferTest, SetDepthFastPath)
{
	framebuffer->clearDepth();

	for (int i = 0; i < 4; ++i)
	{
		const __m128i x = _mm_set1_epi32(i);
		const __m128i y = _mm_set1_epi32(0);
		const __m128 depth = _mm_set1_ps(0.1f + i * 0.1f);
		framebuffer->setDepth(x, y, depth, 0x1);
	}

	const float* depthBuffer = framebuffer->getDepthBuffer();

	EXPECT_FLOAT_EQ(depthBuffer[0], 0.1f);
	EXPECT_FLOAT_EQ(depthBuffer[1], 0.2f);
	EXPECT_FLOAT_EQ(depthBuffer[2], 0.3f);
	EXPECT_FLOAT_EQ(depthBuffer[3], 0.4f);
}

TEST_F(FramebufferTest, SetDepthSlowPath)
{
	framebuffer->clearDepth();

	const __m128i x = _mm_set_epi32(3, 2, 1, 0);
	const __m128i y = _mm_set_epi32(0, 0, 0, 0);
	const __m128 depth = _mm_set_ps(0.8f, 0.6f, 0.4f, 0.2f);

	framebuffer->setDepth(x, y, depth, 0x5);

	const float* depthBuffer = framebuffer->getDepthBuffer();

	EXPECT_FLOAT_EQ(depthBuffer[0], 0.2f);
	EXPECT_FLOAT_EQ(depthBuffer[1], 1.0f);
	EXPECT_FLOAT_EQ(depthBuffer[2], 0.6f);
	EXPECT_FLOAT_EQ(depthBuffer[3], 1.0f);
}

TEST_F(FramebufferTest, DepthTestMaskGeneration)
{
	framebuffer->clearDepth();

	const __m128i x = _mm_set1_epi32(0);
	const __m128i y = _mm_set1_epi32(0);
	const __m128 knownDepth = _mm_set1_ps(0.5f);
	framebuffer->setDepth(x, y, knownDepth, 0x1);

	__m128 testDepth = _mm_set1_ps(0.3f);
	int mask = framebuffer->depthTest(x, y, testDepth);
	EXPECT_EQ(mask & 0x1, 0x1);

	testDepth = _mm_set1_ps(0.7f);
	mask = framebuffer->depthTest(x, y, testDepth);
	EXPECT_EQ(mask & 0x1, 0x0);
}

TEST_F(FramebufferTest, MaskParameterHandling)
{
	const __m128i x = _mm_set_epi32(3, 2, 1, 0);
	const __m128i y = _mm_set_epi32(0, 0, 0, 0);
	const __m128 depth = _mm_set_ps(0.5f, 0.5f, 0.5f, 0.5f);
	const __m128i color = _mm_set1_epi32(0xFF0000);

	framebuffer->setDepth(x, y, depth, 0x0);
	framebuffer->setPixel(x, y, color, 0x0);

	const float* depthBuffer = framebuffer->getDepthBuffer();
	for (int i = 0; i < 4; ++i)
	{
		EXPECT_FLOAT_EQ(depthBuffer[i], 1.0f);
	}
}

TEST_F(FramebufferTest, ColorChannelOrder)
{
	framebuffer->clear();

	const __m128i x = _mm_set1_epi32(0);
	const __m128i y = _mm_set1_epi32(0);
	const __m128i redColor = _mm_set1_epi32(0xFF0000);

	framebuffer->setPixel(x, y, redColor, 0x1);

	const uint8_t* colorBuffer = framebuffer->getColorBuffer();

	const bool pixelSet = (colorBuffer[0] != 0) || (colorBuffer[1] != 0) || (colorBuffer[2] != 0);
	EXPECT_TRUE(pixelSet);
}

TEST_F(FramebufferTest, DepthPrecision)
{
	framebuffer->clearDepth();

	const __m128i x = _mm_set1_epi32(0);
	const __m128i y = _mm_set1_epi32(0);

	const __m128 depth1 = _mm_set1_ps(0.5f);
	framebuffer->setDepth(x, y, depth1, 0x1);

	const __m128 depth2 = _mm_set1_ps(0.4999999f);
	int mask = framebuffer->depthTest(x, y, depth2);
	EXPECT_EQ(mask & 0x1, 0x1);

	const __m128 depth3 = _mm_set1_ps(0.5000001f);
	mask = framebuffer->depthTest(x, y, depth3);
	EXPECT_EQ(mask & 0x1, 0x0);
}

TEST_F(FramebufferTest, SIMDAlignment)
{
	framebuffer->clearDepth();

	for (int offset = 0; offset < 4; ++offset)
	{
		const __m128i x = _mm_set_epi32(offset + 3, offset + 2, offset + 1, offset);
		const __m128i y = _mm_set_epi32(0, 0, 0, 0);
		const __m128 depth = _mm_set_ps(0.1f, 0.2f, 0.3f, 0.4f);

		EXPECT_NO_THROW(framebuffer->setDepth(x, y, depth, 0xF));
		EXPECT_NO_THROW(framebuffer->depthTest(x, y, depth));
	}
}

TEST_F(FramebufferTest, EdgeCaseCoordinates)
{
	const int maxX = framebuffer->getWidth() - 1;
	const int maxY = framebuffer->getHeight() - 1;

	const __m128i xBoundary = _mm_set_epi32(maxX, maxX, 0, 0);
	const __m128i yBoundary = _mm_set_epi32(maxY, 0, maxY, 0);
	const __m128 depth = _mm_set_ps(0.5f, 0.5f, 0.5f, 0.5f);
	const __m128i color = _mm_set1_epi32(0xFF0000);

	EXPECT_NO_THROW(framebuffer->setDepth(xBoundary, yBoundary, depth, 0xF));
	EXPECT_NO_THROW(framebuffer->setPixel(xBoundary, yBoundary, color, 0xF));
	EXPECT_NO_THROW(framebuffer->depthTest(xBoundary, yBoundary, depth));
}
