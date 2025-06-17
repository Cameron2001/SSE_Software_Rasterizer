#include "Window.h"
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <immintrin.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static Window* g_WindowInstance = nullptr;

Window::Window(int width, int height, const std::string& title) : mTitle(title)
{
	if (width <= 0 || height <= 0)
	{
		throw std::invalid_argument("Window dimensions must be positive");
	}
	if (title.empty())
	{
		throw std::invalid_argument("Window title cannot be empty");
	}

	g_WindowInstance = this;

	// register window class
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszClassName = L"SoftwareRendererWindow";

	if (!RegisterClassEx(&wc))
	{
		throw std::runtime_error("Failed to register window class");
	}

	// adjust window size to account for borders
	RECT rect = {0, 0, width, height};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	const int windowWidth = rect.right - rect.left;
	const int windowHeight = rect.bottom - rect.top;

	const std::wstring wTitle(title.begin(), title.end());

	// create window
	mWindowHandle = CreateWindowEx(
		0,
		L"SoftwareRendererWindow",
		wTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr
	);

	if (!mWindowHandle)
	{
		throw std::runtime_error("Failed to create window");
	}

	mDeviceContextHandle = GetDC(mWindowHandle);
	if (!mDeviceContextHandle)
	{
		throw std::runtime_error("Failed to get device context");
	}

	mFrontBuffer = std::make_unique<Framebuffer>(width, height);
	mBackBuffer = std::make_unique<Framebuffer>(width, height);

	// setup DIB for drawing to window
	mBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	mBitmapInfo.bmiHeader.biWidth = width;
	mBitmapInfo.bmiHeader.biHeight = -height; // top-down
	mBitmapInfo.bmiHeader.biPlanes = 1;
	mBitmapInfo.bmiHeader.biBitCount = 24;
	mBitmapInfo.bmiHeader.biCompression = BI_RGB;

	// calculate stride with 4-byte alignment
	const int stride = (width * 3 + 3) / 4 * 4;
	mBitmapInfo.bmiHeader.biSizeImage = stride * height;

	mDIBSection = CreateDIBSection(
		mDeviceContextHandle,
		&mBitmapInfo,
		DIB_RGB_COLORS,
		&mDIBBits,
		nullptr,
		0
	);

	if (!mDIBSection)
	{
		throw std::runtime_error("Failed to create DIB section");
	}

	mDIBDC = CreateCompatibleDC(mDeviceContextHandle);
	if (!mDIBDC)
	{
		DeleteObject(mDIBSection);
		throw std::runtime_error("Failed to create compatible DC");
	}

	SelectObject(mDIBDC, mDIBSection);

	ShowWindow(mWindowHandle, SW_SHOW);
	UpdateWindow(mWindowHandle);
}

Window::~Window()
{
	// clean up GDI resources
	if (mDIBDC)
	{
		DeleteDC(mDIBDC);
		mDIBDC = nullptr;
	}

	if (mDIBSection)
	{
		DeleteObject(mDIBSection);
		mDIBSection = nullptr;
	}

	if (mDeviceContextHandle && mWindowHandle)
	{
		ReleaseDC(mWindowHandle, mDeviceContextHandle);
		mDeviceContextHandle = nullptr;
	}

	if (mWindowHandle)
	{
		DestroyWindow(mWindowHandle);
		mWindowHandle = nullptr;
	}

	UnregisterClass(L"SoftwareRendererWindow", GetModuleHandle(nullptr));

	if (g_WindowInstance == this)
	{
		g_WindowInstance = nullptr;
	}
}

void Window::swapBuffers()
{
	std::swap(mFrontBuffer, mBackBuffer);

	blit();
}

void Window::blit() const
{
	assert(mFrontBuffer && "front buffer not initialized");

	const int width = mFrontBuffer->getWidth();
	const int height = mFrontBuffer->getHeight();
	constexpr int channels = 3; // rgb

	// calculate stride with 4-byte alignment
	const int stride = (width * 3 + 3) / 4 * 4;

	const uint8_t* pixels = mFrontBuffer->getColorBuffer();
	assert(pixels && "invalid framebuffer data");

	if (mDIBSection && mDIBDC)
	{
		const auto dibData = static_cast<uint8_t*>(mDIBBits);
		assert(dibData && "invalid DIB section data");

		//  RGB -> BGR 
		for (int y = 0; y < height; y++)
		{
			const uint8_t* srcRow = &pixels[y * width * channels];
			uint8_t* dstRow = &dibData[y * stride];

			int x = 0;

			// shuffle mask for RGB -> BGR
			const __m128i shuffleMask = _mm_setr_epi8(
				2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 15
			);

			// 4 pixels at a time
			// ensures we don't read past the buffer boundary as 4 RGB pixels = 12 bytes
			for (; x <= width - 4; x += 4)
			{
				const __m128i rgb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&srcRow[x * 3]));

				// Shuffle RGB TO BGR using the shuffle mask
				const __m128i bgr = _mm_shuffle_epi8(rgb, shuffleMask);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(&dstRow[x * 3]), bgr);
			}

			// Handle remaining pixels
			for (; x < width; x++)
			{
				dstRow[x * 3 + 0] = srcRow[x * channels + 2]; // b
				dstRow[x * 3 + 1] = srcRow[x * channels + 1]; // g
				dstRow[x * 3 + 2] = srcRow[x * channels + 0]; // r
			}
		}

		// blit to window
		BitBlt(
			mDeviceContextHandle,
			0, 0,
			width, height,
			mDIBDC,
			0, 0,
			SRCCOPY
		);
	}
}

bool Window::processMessages() const
{
	assert(mWindowHandle && "Window handle should be valid during window lifetime");

	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void Window::setTitle(const std::string& title)
{
	assert(mWindowHandle && "Window handle should be valid during window lifetime");

	if (title.empty())
	{
		std::cerr << "Warning: Setting empty window title\n";
	}

	mTitle = title;

	const std::wstring wTitle(title.begin(), title.end());

	SetWindowText(mWindowHandle, wTitle.c_str());
}

LRESULT CALLBACK WindowProc(const HWND hwnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}
