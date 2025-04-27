#pragma once
#include <string>
#include <cstdint>
#include <immintrin.h>

class Texture
{
public:
	explicit Texture(const std::string& path);
	~Texture();

	bool load(const std::string& path);

	__m128i sample(__m128 u, __m128 v) const;

	bool isLoaded() const { return mIsLoaded; }
	int getWidth() const { return mWidth; }
	int getHeight() const { return mHeight; }
	const uint8_t* getData() const { return mData; }

private:
	int mWidth;
	int mHeight;
	uint8_t* mData;
	bool mIsLoaded;

	bool isInBounds(const int x, const int y) const
	{
		return x >= 0 && x < mWidth && y >= 0 && y < mHeight;
	}
};
