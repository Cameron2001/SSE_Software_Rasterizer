#include "Material.h"
#include <cassert>
#include <utility>

Material::Material() :
	mDiffuseTexture(nullptr)
{
}

void Material::setDiffuseTexture(std::shared_ptr<Texture> texture)
{
	if (texture)
	{
		assert(texture->isLoaded() && "Cannot set unloaded texture as diffuse");
	}

	mDiffuseTexture = std::move(texture);
}
