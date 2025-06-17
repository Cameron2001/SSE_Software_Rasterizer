#include <gtest/gtest.h>
#include "../src/Camera.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

class CameraTest : public testing::Test
{
protected:
	void SetUp() override
	{
		camera = std::make_unique<Camera>();
	}

	std::unique_ptr<Camera> camera;
};

TEST_F(CameraTest, DefaultConstructor)
{
	const Camera testCamera;

	EXPECT_FLOAT_EQ(testCamera.getPosition().x, 0.0f);
	EXPECT_FLOAT_EQ(testCamera.getPosition().y, 0.0f);
	EXPECT_FLOAT_EQ(testCamera.getPosition().z, 0.0f);
	EXPECT_FLOAT_EQ(testCamera.getYaw(), -90.0f);
	EXPECT_FLOAT_EQ(testCamera.getPitch(), 0.0f);
	EXPECT_FLOAT_EQ(testCamera.getFov(), 90.0f);

	const glm::mat4& viewMatrix = testCamera.getViewMatrix();
	const glm::mat4& projMatrix = testCamera.getProjectionMatrix();
	const glm::mat4& viewProjMatrix = testCamera.getViewProjectionMatrix();

	EXPECT_FALSE(viewMatrix == glm::mat4(0.0f));
	EXPECT_FALSE(projMatrix == glm::mat4(0.0f));
	EXPECT_FALSE(viewProjMatrix == glm::mat4(0.0f));
}

TEST_F(CameraTest, ParameterizedConstructor)
{
	constexpr glm::vec3 position(1.0f, 2.0f, 3.0f);
	constexpr glm::vec3 up(0.0f, 1.0f, 0.0f);
	constexpr float yaw = 45.0f;
	constexpr float pitch = 30.0f;
	constexpr float fov = 75.0f;
	constexpr float aspectRatio = 4.0f / 3.0f;
	constexpr float nearPlane = 0.5f;
	constexpr float farPlane = 200.0f;

	const Camera testCamera(position, up, yaw, pitch, fov, aspectRatio, nearPlane, farPlane);

	EXPECT_FLOAT_EQ(testCamera.getPosition().x, position.x);
	EXPECT_FLOAT_EQ(testCamera.getPosition().y, position.y);
	EXPECT_FLOAT_EQ(testCamera.getPosition().z, position.z);
	EXPECT_FLOAT_EQ(testCamera.getYaw(), yaw);
	EXPECT_FLOAT_EQ(testCamera.getPitch(), pitch);
	EXPECT_FLOAT_EQ(testCamera.getFov(), fov);
}

TEST_F(CameraTest, SetPosition)
{
	constexpr glm::vec3 newPosition(5.0f, 10.0f, -3.0f);
	camera->setPosition(newPosition);

	EXPECT_FLOAT_EQ(camera->getPosition().x, newPosition.x);
	EXPECT_FLOAT_EQ(camera->getPosition().y, newPosition.y);
	EXPECT_FLOAT_EQ(camera->getPosition().z, newPosition.z);
}

TEST_F(CameraTest, SetDirection)
{
	constexpr float newYaw = 45.0f;
	constexpr float newPitch = 30.0f;

	camera->setDirection(newYaw, newPitch);

	EXPECT_FLOAT_EQ(camera->getYaw(), newYaw);
	EXPECT_FLOAT_EQ(camera->getPitch(), newPitch);
}

TEST_F(CameraTest, SetFov)
{
	constexpr float newFov = 60.0f;
	camera->setFov(newFov);
	EXPECT_FLOAT_EQ(camera->getFov(), newFov);

	constexpr float anotherNewFov = 75.0f;
	camera->setFov(anotherNewFov);
	EXPECT_FLOAT_EQ(camera->getFov(), anotherNewFov);
}

TEST_F(CameraTest, SetProjectionParams)
{
	constexpr float aspectRatio = 4.0f / 3.0f;
	constexpr float nearPlane = 0.5f;
	constexpr float farPlane = 200.0f;

	const glm::mat4 originalProj = camera->getProjectionMatrix();

	camera->setProjectionParams(aspectRatio, nearPlane, farPlane);

	const glm::mat4& projMatrix = camera->getProjectionMatrix();
	EXPECT_FALSE(projMatrix == glm::mat4(0.0f));
	EXPECT_FALSE(projMatrix == originalProj);
}

TEST_F(CameraTest, PitchClamping)
{
	camera->setDirection(0.0f, 100.0f);
	EXPECT_FLOAT_EQ(camera->getPitch(), 89.0f);

	camera->setDirection(0.0f, -100.0f);
	EXPECT_FLOAT_EQ(camera->getPitch(), -89.0f);

	camera->setDirection(0.0f, 45.0f);
	EXPECT_FLOAT_EQ(camera->getPitch(), 45.0f);
}

TEST_F(CameraTest, DirectionVectorConsistency)
{
	constexpr float yaw = 0.0f;
	constexpr float pitch = 0.0f;
	camera->setDirection(yaw, pitch);

	const glm::vec3& front = camera->getFront();
	EXPECT_NEAR(front.x, 1.0f, 0.01f);
	EXPECT_NEAR(front.y, 0.0f, 0.01f);
	EXPECT_NEAR(front.z, 0.0f, 0.01f);

	camera->setDirection(90.0f, 0.0f);
	const glm::vec3& front90 = camera->getFront();
	EXPECT_NEAR(front90.x, 0.0f, 0.01f);
	EXPECT_NEAR(front90.y, 0.0f, 0.01f);
	EXPECT_NEAR(front90.z, 1.0f, 0.01f);
}

TEST_F(CameraTest, OrthogonalVectors)
{
	const glm::vec3& front = camera->getFront();
	const glm::vec3& up = camera->getUp();
	const glm::vec3& right = camera->getRight();

	EXPECT_NEAR(glm::length(front), 1.0f, 0.01f);
	EXPECT_NEAR(glm::length(up), 1.0f, 0.01f);
	EXPECT_NEAR(glm::length(right), 1.0f, 0.01f);

	EXPECT_NEAR(glm::dot(front, up), 0.0f, 0.01f);
	EXPECT_NEAR(glm::dot(front, right), 0.0f, 0.01f);
	EXPECT_NEAR(glm::dot(up, right), 0.0f, 0.01f);
}

TEST_F(CameraTest, ViewProjectionMatrixConsistency)
{
	const glm::mat4& view = camera->getViewMatrix();
	const glm::mat4& proj = camera->getProjectionMatrix();
	const glm::mat4& viewProj = camera->getViewProjectionMatrix();

	const glm::mat4 expectedViewProj = proj * view;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			EXPECT_NEAR(viewProj[i][j], expectedViewProj[i][j], 0.001f);
		}
	}
}

TEST_F(CameraTest, MatrixUpdateOnPositionChange)
{
	const glm::mat4 originalView = camera->getViewMatrix();
	const glm::mat4 originalViewProj = camera->getViewProjectionMatrix();

	camera->setPosition(glm::vec3(10.0f, 5.0f, -2.0f));

	EXPECT_FALSE(camera->getViewMatrix() == originalView);
	EXPECT_FALSE(camera->getViewProjectionMatrix() == originalViewProj);

	const glm::mat4& proj = camera->getProjectionMatrix();
	EXPECT_FALSE(proj == glm::mat4(0.0f));
}

TEST_F(CameraTest, MatrixUpdateOnDirectionChange)
{
	const glm::mat4 originalView = camera->getViewMatrix();
	const glm::mat4 originalViewProj = camera->getViewProjectionMatrix();

	camera->setDirection(45.0f, 30.0f);

	EXPECT_FALSE(camera->getViewMatrix() == originalView);
	EXPECT_FALSE(camera->getViewProjectionMatrix() == originalViewProj);
}

TEST_F(CameraTest, MatrixUpdateOnFovChange)
{
	const glm::mat4 originalProj = camera->getProjectionMatrix();
	const glm::mat4 originalViewProj = camera->getViewProjectionMatrix();

	camera->setFov(60.0f);

	EXPECT_FALSE(camera->getProjectionMatrix() == originalProj);
	EXPECT_FALSE(camera->getViewProjectionMatrix() == originalViewProj);
}

TEST_F(CameraTest, ValidFovValues)
{
	camera->setFov(0.1f);
	EXPECT_FLOAT_EQ(camera->getFov(), 0.1f);

	camera->setFov(179.9f);
	EXPECT_FLOAT_EQ(camera->getFov(), 179.9f);

	const glm::mat4& projMatrix = camera->getProjectionMatrix();
	EXPECT_FALSE(projMatrix == glm::mat4(0.0f));
}

TEST_F(CameraTest, InvalidProjectionParams)
{
	EXPECT_THROW(camera->setProjectionParams(16.0f / 9.0f, 100.0f, 0.1f), std::invalid_argument); // far < near
	EXPECT_THROW(camera->setProjectionParams(0.0f, 0.1f, 100.0f), std::invalid_argument); // aspect ratio = 0
	EXPECT_THROW(camera->setProjectionParams(-1.0f, 0.1f, 100.0f), std::invalid_argument); // negative aspect ratio
	EXPECT_THROW(camera->setProjectionParams(16.0f / 9.0f, 0.0f, 100.0f), std::invalid_argument); // near plane = 0
	EXPECT_THROW(camera->setProjectionParams(16.0f / 9.0f, 0.1f, 0.0f), std::invalid_argument); // far plane = 0

	EXPECT_NO_THROW(camera->setProjectionParams(16.0f / 9.0f, 0.1f, 100.0f));
	const glm::mat4& projMatrix = camera->getProjectionMatrix();
	EXPECT_FALSE(projMatrix == glm::mat4(0.0f));
}

TEST_F(CameraTest, MatrixNonDegeneracy)
{
	const std::vector<glm::vec3> testPositions = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(100.0f, -50.0f, 25.0f),
		glm::vec3(-1000.0f, 1000.0f, -500.0f)
	};

	const std::vector<std::pair<float, float>> testDirections = {
		{0.0f, 0.0f}, {90.0f, 45.0f}, {-180.0f, -45.0f}, {360.0f, 89.0f}
	};

	for (const auto& pos : testPositions)
	{
		for (const auto& dir : testDirections)
		{
			camera->setPosition(pos);
			camera->setDirection(dir.first, dir.second);

			const glm::mat4& view = camera->getViewMatrix();
			const glm::mat4& proj = camera->getProjectionMatrix();

			EXPECT_FALSE(view == glm::mat4(0.0f));
			EXPECT_FALSE(proj == glm::mat4(0.0f));

			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					EXPECT_FALSE(std::isnan(view[i][j]));
					EXPECT_FALSE(std::isnan(proj[i][j]));
					EXPECT_FALSE(std::isinf(view[i][j]));
					EXPECT_FALSE(std::isinf(proj[i][j]));
				}
			}
		}
	}
}

TEST_F(CameraTest, ExtremePositions)
{
	constexpr glm::vec3 extremePosition(1e6f, -1e6f, 1e6f);
	camera->setPosition(extremePosition);

	EXPECT_FLOAT_EQ(camera->getPosition().x, extremePosition.x);
	EXPECT_FLOAT_EQ(camera->getPosition().y, extremePosition.y);
	EXPECT_FLOAT_EQ(camera->getPosition().z, extremePosition.z);

	const glm::mat4& viewMatrix = camera->getViewMatrix();
	EXPECT_FALSE(viewMatrix == glm::mat4(0.0f));
}

TEST_F(CameraTest, InvalidConstructorParameters)
{
	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.0f), std::invalid_argument);
	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 180.0f), std::invalid_argument);
	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, -10.0f), std::invalid_argument);

	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f, 0.0f),
	             std::invalid_argument);
	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f, -1.0f),
	             std::invalid_argument);

	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f, 1.0f, 0.0f),
	             std::invalid_argument);
	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f, 1.0f, -0.1f),
	             std::invalid_argument);

	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f, 1.0f, 1.0f, 1.0f),
	             std::invalid_argument);
	EXPECT_THROW(Camera(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f, 1.0f, 1.0f, 0.5f),
	             std::invalid_argument);
}

TEST_F(CameraTest, SetFovValidation)
{
	EXPECT_THROW(camera->setFov(0.0f), std::invalid_argument);
	EXPECT_THROW(camera->setFov(180.0f), std::invalid_argument);
	EXPECT_THROW(camera->setFov(-10.0f), std::invalid_argument);
	EXPECT_THROW(camera->setFov(190.0f), std::invalid_argument);

	EXPECT_NO_THROW(camera->setFov(45.0f));
	EXPECT_NO_THROW(camera->setFov(90.0f));
	EXPECT_NO_THROW(camera->setFov(179.9f));
	EXPECT_NO_THROW(camera->setFov(0.1f));
}

TEST_F(CameraTest, SetProjectionParamsValidation)
{
	EXPECT_THROW(camera->setProjectionParams(0.0f, 0.1f, 100.0f), std::invalid_argument);
	EXPECT_THROW(camera->setProjectionParams(-1.0f, 0.1f, 100.0f), std::invalid_argument);

	EXPECT_THROW(camera->setProjectionParams(1.0f, 0.0f, 100.0f), std::invalid_argument);
	EXPECT_THROW(camera->setProjectionParams(1.0f, -0.1f, 100.0f), std::invalid_argument);

	EXPECT_THROW(camera->setProjectionParams(1.0f, 1.0f, 1.0f), std::invalid_argument);
	EXPECT_THROW(camera->setProjectionParams(1.0f, 1.0f, 0.5f), std::invalid_argument);

	EXPECT_NO_THROW(camera->setProjectionParams(16.0f/9.0f, 0.1f, 100.0f));
	EXPECT_NO_THROW(camera->setProjectionParams(1.0f, 0.01f, 1000.0f));
}

TEST_F(CameraTest, SetPositionValidation)
{
	EXPECT_NO_THROW(camera->setPosition(glm::vec3(1.0f, 2.0f, 3.0f)));
	EXPECT_NO_THROW(camera->setPosition(glm::vec3(0.0f, 0.0f, 0.0f)));
	EXPECT_NO_THROW(camera->setPosition(glm::vec3(-1000.0f, 1000.0f, -500.0f)));

	EXPECT_THROW(camera->setPosition(glm::vec3(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f)),
	             std::invalid_argument);
	EXPECT_THROW(camera->setPosition(glm::vec3(0.0f, std::numeric_limits<float>::quiet_NaN(), 0.0f)),
	             std::invalid_argument);
	EXPECT_THROW(camera->setPosition(glm::vec3(0.0f, 0.0f, std::numeric_limits<float>::quiet_NaN())),
	             std::invalid_argument);

	EXPECT_THROW(camera->setPosition(glm::vec3(std::numeric_limits<float>::infinity(), 0.0f, 0.0f)),
	             std::invalid_argument);
	EXPECT_THROW(camera->setPosition(glm::vec3(0.0f, -std::numeric_limits<float>::infinity(), 0.0f)),
	             std::invalid_argument);
	EXPECT_THROW(camera->setPosition(glm::vec3(0.0f, 0.0f, std::numeric_limits<float>::infinity())),
	             std::invalid_argument);
}

TEST_F(CameraTest, SetDirectionPitchWarnings)
{
	EXPECT_NO_THROW(camera->setDirection(0.0f, 45.0f));
	EXPECT_NO_THROW(camera->setDirection(90.0f, -45.0f));
	EXPECT_NO_THROW(camera->setDirection(180.0f, 0.0f));

	EXPECT_NO_THROW(camera->setDirection(0.0f, 100.0f));
	EXPECT_NO_THROW(camera->setDirection(0.0f, -100.0f));
	EXPECT_NO_THROW(camera->setDirection(0.0f, 180.0f));
	EXPECT_NO_THROW(camera->setDirection(0.0f, -180.0f));
}
