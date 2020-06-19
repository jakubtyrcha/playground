#include "os.h"

namespace Os
{
	Window::Window(Vector2i resolution, WndProcCallback wnd_proc) : resolution_(resolution)
	{
		wc_ = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"PlaygroundWndClass", NULL };
		::RegisterClassEx(&wc_);
		hwnd_ = ::CreateWindow(wc_.lpszClassName, L"Playground", WS_OVERLAPPEDWINDOW, 100, 100, resolution_.x(), resolution_.y(), NULL, NULL, wc_.hInstance, NULL);

		::ShowWindow(hwnd_, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd_);
	}

	Window::~Window()
	{
		::DestroyWindow(hwnd_);
		::UnregisterClass(wc_.lpszClassName, wc_.hInstance);
	}

}