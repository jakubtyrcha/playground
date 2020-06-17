#pragma once

#include <d3d12.h>
#include "com.h"

#define DX12_ENABLE_DEBUG_LAYER 1

namespace Gfx
{
	struct Device
	{
		Com::UniqueComPtr<ID3D12Device> device_;

		Device();
		~Device();
	};
}
