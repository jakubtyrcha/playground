#pragma once
#include "Types.h"

namespace Playground
{
	namespace Gfx {
		struct DescriptorHandle {
			uint64_t handle;

			bool operator == (DescriptorHandle) const;
			bool operator != (DescriptorHandle) const;
			operator uintptr_t() const;
		};
	}
}