#pragma once

#include "shared_headers.h"

#include <magnum/MagnumMath.hpp>

using namespace Magnum;

namespace Os
{
	using WndProcCallback = LRESULT(*)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	struct Window
	{
		HWND hwnd_ = 0;
		WNDCLASSEX wc_;
		Vector2i resolution_;

		Window(Vector2i resolution, WndProcCallback wnd_proc);
		~Window();

		template<typename Functor>
		void RunLoop(Functor loop_tick)
		{
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
					continue;
				}

				loop_tick();
			}
		}
	};
}