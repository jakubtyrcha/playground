#pragma once

#include "types.h"

namespace Playground {

namespace Gfx {
    struct Swapchain;
}

namespace Os {
    using WndProcCallback = LRESULT (*)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    struct UserInput {
        bool mouse_down_[4] = {};
        bool keys_down_[512] = {};
    };

    struct Window {
        HWND hwnd_ = 0;
        WNDCLASSEX wc_;
        Vector2i resolution_;
        Gfx::Swapchain* swapchain_ = nullptr;
        UserInput user_input_;
        bool quit_ = false;

        Window(Vector2i resolution, WndProcCallback wnd_proc);
        ~Window();

        void Init();
        void HandleSizeChange(Vector2i resolution);

        bool PumpMessages();
    };

    Window* CreateOsWindow(Vector2i resolution);
    Window* GetWindowByHandle(HWND hwnd);
}

}