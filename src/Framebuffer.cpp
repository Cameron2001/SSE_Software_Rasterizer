#include "Framebuffer.h"
#include <algorithm>
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
	alignas(16) uint8_t c[16]; // 16 bytes for alignment, using 3 per pixel (rgb)
	_mm_store_si128((__m128i*)xs, x);
	_mm_store_si128((__m128i*)ys, y);
	_mm_store_si128((__m128i*)c, color);

	// process only pixels enabled by mask bits
	for (int i = 0; i < 4; ++i)
		if (mask & (1 << i))
		{
			const int x = xs[i];
			const int y = ys[i];
			if (!isInBounds(x, y)) continue;

			// calculate pixel offset (y * width + x) * 3 bytes
			const size_t idx = (static_cast<size_t>(y) * mWidth + x) * 3;
			// safety check to prevent out of bounds memory access
			if (idx + 2 < mPixels.size())
			{
				// write rgb color
				mPixels[idx] = c[i * 4]; // r
				mPixels[idx + 1] = c[i * 4 + 1]; // g
				mPixels[idx + 2] = c[i * 4 + 2]; // b
			}
		}
}


void Framebuffer::setDepth(const __m128i xI, const __m128i yI, const __m128 depth, const int mask)
{
	if (!mask) return;

	// fast path: all 4 pixels are valid
	if (mask == 0xF)
	{
		const int x0 = _mm_cvtsi128_si32(xI);
		const int y0 = _mm_cvtsi128_si32(yI);

		if (!isInBounds(x0, y0) || !isInBounds(x0 + 3, y0)) return;

		// write all 4 depth values in one simd operation
		const size_t base = y0 * mWidth + x0;
		_mm_store_ps(mDepthBuffer.data() + base, depth);
		return;
	}

	// slow path: handle individual pixels
	alignas(16) int xs[4], ys[4];
	alignas(16) float ds[4];
	_mm_store_si128((__m128i*)xs, xI);
	_mm_store_si128((__m128i*)ys, yI);
	_mm_store_ps(ds, depth);

	for (int i = 0; i < 4; ++i)
		if (mask & (1 << i))
		{
			if (!isInBounds(xs[i], ys[i])) continue;

			const size_t index = ys[i] * mWidth + xs[i];
			mDepthBuffer[index] = ds[i];
		}
}


int Framebuffer::depthTest(const __m128i x, const __m128i y, const __m128 depth) const
{
	// calculate buffer indices: y * width + x
	const __m128i idx = _mm_add_epi32(_mm_mullo_epi32(y, _mm_set1_epi32(mWidth)), x);

	// load existing depth values 
	alignas(16) int xs[4];
	_mm_store_si128((__m128i*)xs, idx);

	const __m128 curr = _mm_loadu_ps(mDepthBuffer.data() + xs[0]);

	// depth test
	const __m128 cmp = _mm_cmplt_ps(depth, curr);

	// return 4-bit mask
	return _mm_movemask_ps(cmp);
}
