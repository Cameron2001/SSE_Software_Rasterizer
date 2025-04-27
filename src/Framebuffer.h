#pragma once
#include <emmintrin.h>
#include <vector>
#include <cstdint>

class Framebuffer
{
public:
	Framebuffer(int w, int h);

	void clear();
	void clearDepth();

	void setPixel(__m128i x, __m128i y, __m128i color, int mask);
	void setDepth(__m128i x, __m128i y, __m128 depth, int mask);
	int depthTest(__m128i x, __m128i y, __m128 depth) const;

	int getWidth() const { return mWidth; }
	int getHeight() const { return mHeight; }
	const uint8_t* getColorBuffer() const { return mPixels.data(); }
	const float* getDepthBuffer() const { return mDepthBuffer.data(); }

private:
	int mWidth;
	int mHeight;

	std::vector<uint8_t> mPixels;
	std::vector<float> mDepthBuffer;

	bool isInBounds(const int x, const int y) const
	{
		return x >= 0 && x < mWidth && y >= 0 && y < mHeight;
	}
};
