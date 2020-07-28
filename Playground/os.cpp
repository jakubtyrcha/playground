#include "pch.h"

#include "assertions.h"
#include "imgui_impl_win32.h"
#include "os.h"

#include "array.h"
#include "box.h"
#include "gfx.h"

#include <imgui/imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Playground {
namespace Os {
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    Window::Window(Vector2i resolution, WndProcCallback wnd_proc)
        : resolution_(resolution)
    {
        wc_ = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"PlaygroundWndClass", NULL };
        ::RegisterClassEx(&wc_);
        hwnd_ = ::CreateWindow(wc_.lpszClassName, L"Playground", WS_OVERLAPPEDWINDOW, 100, 100, resolution_.x(), resolution_.y(), NULL, NULL, wc_.hInstance, NULL);
    }

    Window::~Window()
    {
        ::DestroyWindow(hwnd_);
        ::UnregisterClass(wc_.lpszClassName, wc_.hInstance);
    }

    void Window::Init()
    {
        ::ShowWindow(hwnd_, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd_);
    }

    void Window::HandleSizeChange(Vector2i resolution)
    {
        resolution_ = resolution;
    }

    Array<Box<Window>> GlobalWindowsList;

    Window* CreateOsWindow(Vector2i resolution)
    {
        GlobalWindowsList.PushBackRvalueRef(MakeBox<Window>(resolution, WndProc));
        return GlobalWindowsList.Last().get();
    }

    Window* GetWindowByHandle(HWND hwnd)
    {
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

        Window* wnd = Os::GetWindowByHandle(hWnd);

        if (wnd) {
            switch (msg) {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONDBLCLK:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONDBLCLK: {
                int button = 0;
                if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) {
                    button = 0;
                }
                if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) {
                    button = 1;
                }
                if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) {
                    button = 2;
                }
                wnd->user_input_.mouse_down_[button] = true;
                return 0;
            }
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP: {
                int button = 0;
                if (msg == WM_LBUTTONUP) {
                    button = 0;
                }
                if (msg == WM_RBUTTONUP) {
                    button = 1;
                }
                if (msg == WM_MBUTTONUP) {
                    button = 2;
                }
                wnd->user_input_.mouse_down_[button] = false;
                return 0;
            }

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                if (wParam < 256) {
                    wnd->user_input_.keys_down_[wParam] = 1;
                }
                return 0;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (wParam < 256) {
                    wnd->user_input_.keys_down_[wParam] = 0;
                }
                return 0;
            case WM_SIZE:
                if (wParam != SIZE_MINIMIZED) {
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
            }
        }

        switch (msg) {
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
}