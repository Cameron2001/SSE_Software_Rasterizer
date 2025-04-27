#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <filesystem>

Texture::Texture(const std::string& path) : mWidth(0), mHeight(0), mData(nullptr), mIsLoaded(false)
{
	if (path.empty())
	{
		std::cerr << "Texture path is empty" << '\n';
		return;
	}

	if (!std::filesystem::exists(path))
	{
		std::cerr << "Texture file does not exist: " << path << '\n';
		return;
	}

	load(path);
}

Texture::~Texture()
{
	if (mData)
	{
		stbi_image_free(mData);
		mData = nullptr;
	}
}

bool Texture::load(const std::string& path)
{
	// free existing data if any
	if (mData)
	{
		stbi_image_free(mData);
		mData = nullptr;
	}

	if (path.empty())
	{
		std::cerr << "Cannot load texture from empty path" << '\n';
		return false;
	}

	// load image using stb_image (forces RGB format)
	int width, height, channels;
	mData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);
	if (!mData)
	{
		std::cerr << "Failed to load texture: " << path << '\n';
		std::cerr << "Reason: " << stbi_failure_reason() << '\n';
		mIsLoaded = false;
		return false;
	}

	if (width <= 0 || height <= 0)
	{
		std::cerr << "Invalid texture dimensions: " << width << "x" << height << '\n';
		stbi_image_free(mData);
		mData = nullptr;
		mIsLoaded = false;
		return false;
	}

	mWidth = width;
	mHeight = height;
	mIsLoaded = true;
	return true;
}

__m128i Texture::sample(__m128 u, __m128 v) const
{
	// fallback color if texture not loaded
	if (!mIsLoaded || !mData)
	{
		return _mm_set1_epi32(0x00FFFF);
	}

	__m128 zero = _mm_setzero_ps();
	__m128 one = _mm_set1_ps(1.0f);

	// clamp uv  to [0,1]
	u = _mm_max_ps(zero, _mm_min_ps(u, one));
	v = _mm_max_ps(zero, _mm_min_ps(v, one));

	// convert from uv space to pixel space 
	__m128 wMinus1 = _mm_set1_ps(static_cast<float>(mWidth - 1));
	__m128 hMinus1 = _mm_set1_ps(static_cast<float>(mHeight - 1));
	__m128 uScaled = _mm_mul_ps(u, wMinus1);
	__m128 vScaled = _mm_mul_ps(v, hMinus1);

	// convert float coords to int pixel indices 
	__m128i xi = _mm_cvttps_epi32(uScaled);
	__m128i yi = _mm_cvttps_epi32(vScaled);

	// calculate pixel indices (y * width + x)
	__m128i widthVec = _mm_set1_epi32(mWidth);
	__m128i yMulW = _mm_mullo_epi32(yi, widthVec);
	__m128i idx = _mm_add_epi32(yMulW, xi);

	// multiply by 3 for RGB pixel data
	__m128i idx2 = _mm_mullo_epi32(idx, _mm_set1_epi32(3));


	alignas(16) int indices[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(indices), idx2);

	// load and pack RGB 
	alignas(16) uint32_t colors[4];
	for (int i = 0; i < 4; i++)
	{
		int index = indices[i];
		uint8_t r = mData[index];
		uint8_t g = mData[index + 1];
		uint8_t b = mData[index + 2];
		colors[i] = (b << 16) | (g << 8) | r;
	}

	return _mm_load_si128(reinterpret_cast<const __m128i*>(colors));
}
