#include "Framebuffer.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <smmintrin.h>


Framebuffer::Framebuffer(const int w, const int h)
	: mWidth(w)
	  , mHeight(h)
{
	if (w <= 0 || h <= 0)
	{
		throw std::invalid_argument("Framebuffer dimensions must be positive");
	}

	// allocate rgb buffer (3 bytes/pixel) and depth buffer
	mPixels.resize(static_cast<size_t>(mWidth) * mHeight * 3, 0);
	mDepthBuffer.resize(static_cast<size_t>(mWidth) * mHeight, 1.0f);
}

void Framebuffer::clear()
{
	std::ranges::fill(mPixels, 0);
}

void Framebuffer::clearDepth()
{
	std::ranges::fill(mDepthBuffer, 1.0f);
}


void Framebuffer::setPixel(const __m128i x, const __m128i y, const __m128i color, const int mask)
{
	if (mask == 0) return;

	alignas(16) int xs[4], ys[4];
	alignas(16) uint8_t c[16];
	_mm_store_si128((__m128i*)xs, x);
	_mm_store_si128((__m128i*)ys, y);
	_mm_store_si128((__m128i*)c, color);

	for (int i = 0; i < 4; ++i)
		if (mask & (1 << i))
		{
			const int px = xs[i];
			const int py = ys[i];

			assert(isInBounds(px, py) && "Pixel coordinates out of bounds");

			const size_t idx = (static_cast<size_t>(py) * mWidth + px) * 3;
			assert(idx + 2 < mPixels.size() && "Pixel index out of bounds");

			mPixels[idx] = c[i * 4]; // r
			mPixels[idx + 1] = c[i * 4 + 1]; // g
			mPixels[idx + 2] = c[i * 4 + 2]; // b
		}
}

void Framebuffer::setDepth(const __m128i xI, const __m128i yI, const __m128 depth, const int mask)
{
	assert(mask >= 0 && mask <= 0xF && "Invalid mask value");
	if (mask == 0) return;

	// fast path: all 4 pixels
	if (mask == 0xF)
	{
		const int x0 = _mm_cvtsi128_si32(xI);
		const int y0 = _mm_cvtsi128_si32(yI);
		assert(isInBounds(x0, y0) && isInBounds(x0 + 3, y0) && "Pixel coordinates out of bounds");

		const size_t base = static_cast<size_t>(y0) * mWidth + x0;
		assert(base + 3 < mDepthBuffer.size() && "Depth buffer index out of bounds");
		_mm_storeu_ps(mDepthBuffer.data() + base, depth);
		return;
	}

	// slow path: individual pixels
	alignas(16) int xs[4], ys[4];
	alignas(16) float ds[4];
	_mm_store_si128((__m128i*)xs, xI);
	_mm_store_si128((__m128i*)ys, yI);
	_mm_store_ps(ds, depth);

	for (int i = 0; i < 4; ++i)
		if (mask & (1 << i))
		{
			assert(isInBounds(xs[i], ys[i]) && "Pixel coordinates out of bounds");
			const size_t index = static_cast<size_t>(ys[i]) * mWidth + xs[i];
			assert(index < mDepthBuffer.size() && "Depth buffer index out of bounds");
			mDepthBuffer[index] = ds[i];
		}
}

int Framebuffer::depthTest(const __m128i x, const __m128i y, const __m128 depth) const
{
	// calculate buffer indices: y * width + x
	const __m128i idxVec = _mm_add_epi32(_mm_mullo_epi32(y, _mm_set1_epi32(mWidth)), x);

	alignas(16) int idxArr[4];
	_mm_store_si128((__m128i*)idxArr, idxVec);
	// validate indices for internal consistency
	for (int i = 0; i < 4; ++i)
		assert(
		idxArr[i] >= 0 && static_cast<size_t>(idxArr[i]) < mDepthBuffer.size() && "Depth buffer index out of bounds");

	const __m128 curr = _mm_loadu_ps(mDepthBuffer.data() + idxArr[0]);

	// depth test
	const __m128 cmp = _mm_cmplt_ps(depth, curr);
	return _mm_movemask_ps(cmp);
}
