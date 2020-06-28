#pragma once

#include "types.h"
#include "copy_move.h"

namespace Random {
	struct Generator : private Pinned<Generator> {
		void * state_ = nullptr;

		Generator(u32 seed = 0);
		~Generator();

		u32 U32Uniform();
		f32 F32Uniform(); // [0,1)
		f32 F32UniformInRange(f32 from, f32 to);
	};
}