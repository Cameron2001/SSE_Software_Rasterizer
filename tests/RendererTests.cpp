#include <gtest/gtest.h>
#include "../src/Renderer.h"
#include "../src/Camera.h"
#include "../src/Model.h"
#include "../src/Framebuffer.h"

class RendererTest : public testing::Test
{
protected:
	void SetUp() override
	{
		renderer = std::make_unique<Renderer>();
		framebuffer = std::make_unique<Framebuffer>(640, 480);
		camera = std::make_unique<Camera>(
			glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			-90.0f, 0.0f, 45.0f,
			640.0f / 480.0f, 0.1f, 100.0f
		);

		createTestModel();
	}

	void createTestModel()
	{
		VertexArray vertexArray;
		vertexArray.resize(3);

		vertexArray.positionsX = {0.0f, -1.0f, 1.0f};
		vertexArray.positionsY = {1.0f, -1.0f, -1.0f};
		vertexArray.positionsZ = {0.0f, 0.0f, 0.0f};

		vertexArray.uvsU = {0.5f, 0.0f, 1.0f};
		vertexArray.uvsV = {0.0f, 1.0f, 1.0f};

		vertexArray.normalsX = {0.0f, 0.0f, 0.0f};
		vertexArray.normalsY = {0.0f, 0.0f, 0.0f};
		vertexArray.normalsZ = {1.0f, 1.0f, 1.0f};

		Mesh mesh(vertexArray);
		std::vector<Mesh> meshes = {mesh};
		model = std::make_unique<Model>(meshes);
	}

	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Framebuffer> framebuffer;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Model> model;
};

TEST_F(RendererTest, Construction)
{
	EXPECT_NE(renderer, nullptr);
}

TEST_F(RendererTest, RenderModelBasic)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderWithDifferentCameraPositions)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	camera->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	camera->setPosition(glm::vec3(2.0f, 2.0f, 2.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	camera->setPosition(glm::vec3(-1.0f, -1.0f, 1.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderWithDifferentModelTransforms)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	model->setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	model->setRotation(glm::vec3(0.0f, 45.0f, 0.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	model->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderWithVerySmallFramebuffer)
{
	auto smallFramebuffer = std::make_unique<Framebuffer>(1, 1);
	smallFramebuffer->clear();
	smallFramebuffer->clearDepth();

	EXPECT_NO_THROW(renderer->renderModel(*smallFramebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderWithLargeFramebuffer)
{
	auto largeFramebuffer = std::make_unique<Framebuffer>(1920, 1080);
	largeFramebuffer->clear();
	largeFramebuffer->clearDepth();

	EXPECT_NO_THROW(renderer->renderModel(*largeFramebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderMultipleMeshModel)
{
	VertexArray vertexArray1, vertexArray2;

	vertexArray1.resize(3);
	vertexArray1.positionsX = {0.0f, 1.0f, -1.0f};
	vertexArray1.positionsY = {1.0f, -1.0f, -1.0f};
	vertexArray1.positionsZ = {0.0f, 0.0f, 0.0f};
	vertexArray1.uvsU = {0.5f, 1.0f, 0.0f};
	vertexArray1.uvsV = {0.0f, 1.0f, 1.0f};
	vertexArray1.normalsX = {0.0f, 0.0f, 0.0f};
	vertexArray1.normalsY = {0.0f, 0.0f, 0.0f};
	vertexArray1.normalsZ = {1.0f, 1.0f, 1.0f};

	vertexArray2.resize(3);
	vertexArray2.positionsX = {0.5f, 1.5f, -0.5f};
	vertexArray2.positionsY = {1.5f, -0.5f, -0.5f};
	vertexArray2.positionsZ = {-1.0f, -1.0f, -1.0f};
	vertexArray2.uvsU = {0.5f, 1.0f, 0.0f};
	vertexArray2.uvsV = {0.0f, 1.0f, 1.0f};
	vertexArray2.normalsX = {0.0f, 0.0f, 0.0f};
	vertexArray2.normalsY = {0.0f, 0.0f, 0.0f};
	vertexArray2.normalsZ = {1.0f, 1.0f, 1.0f};

	Mesh mesh1(vertexArray1);
	Mesh mesh2(vertexArray2);
	std::vector<Mesh> meshes = {mesh1, mesh2};

	Model multiMeshModel(meshes);

	framebuffer->clear();
	framebuffer->clearDepth();

	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, multiMeshModel));
}

TEST_F(RendererTest, RenderWithExtremeTransforms)
{
	framebuffer->clear();
	framebuffer->clearDepth();
	model->setPosition(glm::vec3(0.0f, 0.0f, -50.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
	model->setPosition(glm::vec3(0.0f, 0.0f, 0.2f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
	model->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	model->setScale(glm::vec3(100.0f, 100.0f, 100.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
	model->setScale(glm::vec3(0.01f, 0.01f, 0.01f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderOffscreenModel)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	model->setPosition(glm::vec3(1000.0f, 1000.0f, 1000.0f));

	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderWithDifferentCameraAngles)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	camera->setDirection(0.0f, 0.0f);
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	camera->setDirection(90.0f, 0.0f);
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	camera->setDirection(180.0f, 0.0f);
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	camera->setDirection(0.0f, 45.0f);
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));

	camera->setDirection(0.0f, -45.0f);
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, ConsecutiveRenders)
{
	for (int i = 0; i < 5; ++i)
	{
		framebuffer->clear();
		framebuffer->clearDepth();

		model->setRotation(glm::vec3(0.0f, i * 30.0f, 0.0f));
		EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
	}
}

TEST_F(RendererTest, RenderPipelineValidation)
{
	framebuffer->clear();
	framebuffer->clearDepth();

	const uint8_t* colorBuffer = framebuffer->getColorBuffer();
	bool initiallyEmpty = true;
	for (int i = 0; i < 100 && initiallyEmpty; ++i)
	{
		if (colorBuffer[i] != 0) initiallyEmpty = false;
	}
	EXPECT_TRUE(initiallyEmpty);

	model->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	model->setScale(glm::vec3(1.0f, 1.0f, 1.0f));
	camera->setPosition(glm::vec3(0.0f, 0.0f, 2.0f));

	renderer->renderModel(*framebuffer, *camera, *model);
	const uint8_t* colorBufferAfter = framebuffer->getColorBuffer();
	bool hasRenderedContent = false;
	for (int i = 0; i < framebuffer->getWidth() * framebuffer->getHeight() * 3; ++i)
	{
		if (colorBufferAfter[i] != 0)
		{
			hasRenderedContent = true;
			break;
		}
	}

	EXPECT_TRUE(hasRenderedContent);
}

TEST_F(RendererTest, CullingAndClipping)
{
	framebuffer->clear();
	framebuffer->clearDepth();
	model->setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
	camera->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
	model->setPosition(glm::vec3(100.0f, 0.0f, -5.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
	model->setPosition(glm::vec3(0.0f, 0.0f, -5.0f));
	model->setScale(glm::vec3(1000.0f, 1000.0f, 1000.0f));
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, *model));
}

TEST_F(RendererTest, RenderWithCorruptedData)
{
	framebuffer->clear();
	framebuffer->clearDepth();
	VertexArray extremeArray;
	extremeArray.resize(3);
	extremeArray.positionsX = {1e20f, -1e20f, 0.0f};
	extremeArray.positionsY = {1e20f, -1e20f, 0.0f};
	extremeArray.positionsZ = {1e20f, -1e20f, 0.0f};
	extremeArray.uvsU = {0.0f, 1.0f, 0.5f};
	extremeArray.uvsV = {0.0f, 1.0f, 0.5f};
	extremeArray.normalsX = {0.0f, 0.0f, 0.0f};
	extremeArray.normalsY = {0.0f, 0.0f, 0.0f};
	extremeArray.normalsZ = {1.0f, 1.0f, 1.0f};

	Mesh extremeMesh(extremeArray);
	std::vector<Mesh> extremeMeshes = {extremeMesh};
	Model extremeModel(extremeMeshes);
	EXPECT_NO_THROW(renderer->renderModel(*framebuffer, *camera, extremeModel));
}
