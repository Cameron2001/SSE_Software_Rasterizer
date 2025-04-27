#include <iostream>
#include <string>
#include <format>
#include <chrono>
#include <filesystem>
#include "Framebuffer.h"
#include "Model.h"
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"


int main()
{
	try
	{
		constexpr int WIDTH = 1920;
		constexpr int HEIGHT = 1080;
		constexpr float ASPECT_RATIO = static_cast<float>(WIDTH) / HEIGHT;
		const std::string TITLE = "Software Renderer";

		Window window(WIDTH, HEIGHT, TITLE);

		Renderer renderer;
		Model cube("../assets/sammax.obj");
		cube.setScale(glm::vec3(2.0f, 2.0f, 2.0f));
		cube.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		cube.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));

		Camera camera(
			glm::vec3(0.0f, 1.5f, 3.0f), // position
			glm::vec3(0.0f, 1.0f, 0.0f), // up vector
			-90.0f, // yaw
			0.0f, // pitch
			90.0f, // fov
			ASPECT_RATIO, // aspect ratio
			0.1f, // near plane
			100.0f // far plane
		);

		int frameCount = 0;
		double fps = 0.0;
		auto lastFpsUpdateTime = std::chrono::high_resolution_clock::now();
		auto lastFrameTime = lastFpsUpdateTime;

		bool running = true;
		while (running)
		{
			float rotationSpeed = 30.0f;
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto frameTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
			lastFrameTime = currentTime;

			frameCount++;
			auto fpsElapsedTime = std::chrono::duration<float>(currentTime - lastFpsUpdateTime).count();
			if (fpsElapsedTime >= 1.0f)
			{
				fps = frameCount / fpsElapsedTime;
				frameCount = 0;
				lastFpsUpdateTime = currentTime;

				std::string titleWithFps =
					std::format("{} - FPS: {:.1f} - Frame Time: {:.2f} ms",
					            TITLE, fps, frameTime * 1000.0f);
				window.setTitle(titleWithFps);
			}

			running = window.processMessages();

			Framebuffer* backBuffer = window.getBackBuffer();
			assert(backBuffer && "Back buffer is null");
			backBuffer->clear();
			backBuffer->clearDepth();

			// rotate model
			glm::vec3 rotation = cube.getRotation();
			rotation.y += rotationSpeed * frameTime;
			cube.setRotation(rotation);

			renderer.renderModel(*backBuffer, camera, cube);

			window.swapBuffers();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "Unknown error occurred\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
