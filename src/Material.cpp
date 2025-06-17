#include "Material.h"
#include <stdexcept>
#include <iostream>
#include <utility>
#include <cassert>

Material::Material() :
	mDiffuseTexture(nullptr)
{
}

void Material::setDiffuseTexture(std::shared_ptr<Texture> texture)
{
	if (!texture)
	{
		throw std::invalid_argument("Texture cannot be null");
	}

	if (!texture->isLoaded())
	{
		throw std::invalid_argument("Cannot set unloaded texture as diffuse material");
	}

	mDiffuseTexture = std::move(texture);
}
