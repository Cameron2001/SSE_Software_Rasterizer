#include <gtest/gtest.h>
#include "../src/Material.h"
#include "../src/Texture.h"
#include <memory>

class MaterialTest : public testing::Test
{
protected:
	void SetUp() override
	{
		material = std::make_unique<Material>();
	}

	std::unique_ptr<Material> material;
};

TEST_F(MaterialTest, DefaultConstruction)
{
	EXPECT_EQ(material->getDiffuseTexture(), nullptr);
}

TEST_F(MaterialTest, SetNullTexture)
{
	EXPECT_THROW(material->setDiffuseTexture(nullptr), std::invalid_argument);
}

TEST_F(MaterialTest, SetValidTexture)
{
	const auto texture = std::make_shared<Texture>("../assets/material_baseColor.png");

	if (texture->isLoaded())
	{
		material->setDiffuseTexture(texture);
		EXPECT_EQ(material->getDiffuseTexture(), texture.get());
	}
	else
	{
		EXPECT_EQ(material->getDiffuseTexture(), nullptr);
	}
}

TEST_F(MaterialTest, SetUnloadedTextureThrows)
{
	const auto unloadedTexture = std::make_shared<Texture>("");

	EXPECT_FALSE(unloadedTexture->isLoaded());
	EXPECT_THROW(material->setDiffuseTexture(unloadedTexture), std::invalid_argument);
}

TEST_F(MaterialTest, ValidTextureOperations)
{
	const auto validTexture = std::make_shared<Texture>("../assets/material_baseColor.png");

	EXPECT_THROW(material->setDiffuseTexture(nullptr), std::invalid_argument);

	if (validTexture->isLoaded())
	{
		material->setDiffuseTexture(validTexture);
		EXPECT_EQ(material->getDiffuseTexture(), validTexture.get());

		const auto anotherTexture = std::make_shared<Texture>("../assets/material_2_baseColor.png");
		if (anotherTexture->isLoaded())
		{
			material->setDiffuseTexture(anotherTexture);
			EXPECT_EQ(material->getDiffuseTexture(), anotherTexture.get());
		}
	}
	else
	{
		EXPECT_THROW(material->setDiffuseTexture(validTexture), std::invalid_argument);
	}
}

TEST_F(MaterialTest, SharedTextureOwnership)
{
	Material material1, material2;

	const auto sharedTexture = std::make_shared<Texture>("../assets/material_baseColor.png");

	if (sharedTexture->isLoaded())
	{
		material1.setDiffuseTexture(sharedTexture);
		material2.setDiffuseTexture(sharedTexture);

		EXPECT_EQ(material1.getDiffuseTexture(), sharedTexture.get());
		EXPECT_EQ(material2.getDiffuseTexture(), sharedTexture.get());
		EXPECT_EQ(material1.getDiffuseTexture(), material2.getDiffuseTexture());
	}
	else
	{
		EXPECT_THROW(material1.setDiffuseTexture(nullptr), std::invalid_argument);
		EXPECT_THROW(material2.setDiffuseTexture(nullptr), std::invalid_argument);

		EXPECT_THROW(material1.setDiffuseTexture(sharedTexture), std::invalid_argument);
		EXPECT_THROW(material2.setDiffuseTexture(sharedTexture), std::invalid_argument);
	}
}
