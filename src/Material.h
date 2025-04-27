#pragma once
#include <memory>
#include "Texture.h"

class Material
{
public:
	Material();

	void setDiffuseTexture(std::shared_ptr<Texture> texture);
	const Texture* getDiffuseTexture() const { return mDiffuseTexture.get(); }

private:
	std::shared_ptr<Texture> mDiffuseTexture;
};
