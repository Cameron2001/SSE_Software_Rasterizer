#include <gtest/gtest.h>
#include "../src/Window.h"

class WindowTest : public testing::Test
{
protected:
	void SetUp() override
	{
		width = 800;
		height = 600;
		title = "Test Window";
	}

	int width;
	int height;
	std::string title;
};

TEST_F(WindowTest, Construction)
{
	EXPECT_NO_THROW(const Window window(width, height, title));
}

TEST_F(WindowTest, DimensionValidation)
{
	EXPECT_THROW(const Window window(0, height, title), std::invalid_argument);
	EXPECT_THROW(const Window window(width, 0, title), std::invalid_argument);
	EXPECT_THROW(const Window window(-1, height, title), std::invalid_argument);
	EXPECT_THROW(const Window window(width, -1, title), std::invalid_argument);

	EXPECT_NO_THROW(const Window window(1, 1, "Tiny Window"));
	EXPECT_NO_THROW(const Window window(1920, 1080, "Large Window"));
}

TEST_F(WindowTest, TitleOperations)
{
	Window window(width, height, title);

	EXPECT_NO_THROW(window.setTitle("New Test Title"));

	EXPECT_NO_THROW(window.setTitle(""));

	const std::string specialTitle = "Test αβγ";
	EXPECT_NO_THROW(window.setTitle(specialTitle));

	const std::string longTitle(500, 'A');
	EXPECT_NO_THROW(window.setTitle(longTitle));
}

TEST_F(WindowTest, CoreFunctionality)
{
	Window window(width, height, title);

	const Framebuffer* backBuffer1 = window.getBackBuffer();
	const Framebuffer* backBuffer2 = window.getBackBuffer();

	EXPECT_NE(backBuffer1, nullptr);
	EXPECT_EQ(backBuffer1, backBuffer2);
	EXPECT_EQ(backBuffer1->getWidth(), width);
	EXPECT_EQ(backBuffer1->getHeight(), height);

	EXPECT_NO_THROW(window.swapBuffers());
	EXPECT_NO_THROW(window.processMessages());
	EXPECT_NO_THROW(window.blit());

	const Framebuffer* backBufferAfter = window.getBackBuffer();
	EXPECT_NE(backBufferAfter, nullptr);
	EXPECT_EQ(backBufferAfter->getWidth(), width);
	EXPECT_EQ(backBufferAfter->getHeight(), height);
}

TEST_F(WindowTest, TitleValidation)
{
	EXPECT_THROW(const Window window(width, height, ""), std::invalid_argument);

	EXPECT_NO_THROW(const Window window(width, height, "Valid Title"));
	EXPECT_NO_THROW(const Window window(width, height, "T"));
}
