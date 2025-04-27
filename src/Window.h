#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include "Framebuffer.h"

class Window
{
public:
	Window(int width, int height, const std::string& title);
	~Window();

	void swapBuffers();
	void blit() const;
	bool processMessages() const;
	void setTitle(const std::string& title);

	Framebuffer* getBackBuffer() const { return mBackBuffer.get(); }

private:
	std::string mTitle;
	HWND mWindowHandle = nullptr;
	HDC mDeviceContextHandle = nullptr;

	std::unique_ptr<Framebuffer> mFrontBuffer;
	std::unique_ptr<Framebuffer> mBackBuffer;

	HBITMAP mDIBSection = nullptr;
	HDC mDIBDC = nullptr;
	void* mDIBBits = nullptr;
	BITMAPINFO mBitmapInfo = {};
};
