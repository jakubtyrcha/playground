#include "pch.h"

#include "assertions.h"
#include "os.h"
#include "imgui_impl_win32.h"

#include "array.h"
#include "box.h"
#include "gfx.h"

#include <imgui/imgui.h>

using namespace Containers;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Os
{
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	Window::Window(Vector2i resolution, WndProcCallback wnd_proc) : resolution_(resolution) {
		wc_ = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"PlaygroundWndClass", NULL };
		::RegisterClassEx(&wc_);
		hwnd_ = ::CreateWindow(wc_.lpszClassName, L"Playground", WS_OVERLAPPEDWINDOW, 100, 100, resolution_.x(), resolution_.y(), NULL, NULL, wc_.hInstance, NULL);
	}

	Window::~Window() {
		::DestroyWindow(hwnd_);
		::UnregisterClass(wc_.lpszClassName, wc_.hInstance);
	}

	void Window::Init() {
		::ShowWindow(hwnd_, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd_);
	}

	void Window::HandleSizeChange(Vector2i resolution) {
		resolution_ = resolution;
	}

	Array<Box<Window>> GlobalWindowsList;

	Window* CreateOsWindow(Vector2i resolution) {
		GlobalWindowsList.PushBackRvalueRef(MakeBox<Window>(resolution, WndProc));
		return GlobalWindowsList.Last().get();
	}

	Window* GetWindowByHandle(HWND hwnd) {
		for (Box<Window>& wnd : GlobalWindowsList) {
			if (wnd->hwnd_ == hwnd) {
				return wnd.get();
			}
		}
		return nullptr;
	}

	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
			{
				Window* wnd = Os::GetWindowByHandle(hWnd);
				assert(wnd);
				Gfx::Swapchain* swapchain = wnd->swapchain_;
				assert(swapchain);
				Gfx::Device* device = swapchain->device_;
				assert(device);
				device->GetWaitable().Wait();
				swapchain->Destroy();
				wnd->HandleSizeChange({ static_cast<i32>(LOWORD(lParam)), static_cast<i32>(HIWORD(lParam)) });
				swapchain->Recreate();
			}
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
}