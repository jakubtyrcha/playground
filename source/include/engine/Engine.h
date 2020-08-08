#include "Gfx.h"

namespace Playground {

namespace Engine {
    void Start();
    void Shutdown();
    void FrameBegin();
}

namespace Gfx {

    Swapchain* CreateWindowAndSwapchain(Device* device, Vector2i resolution, i32 backbuffers);
}

}