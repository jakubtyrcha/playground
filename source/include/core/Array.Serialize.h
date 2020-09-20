#pragma once

#include "Array.h"

#include <cereal/cereal.hpp>

namespace Playground {

template <class Archive, typename T>
void save(Archive& ar, Array<T> const& o)
{
	ar(o.size_);
	for (i32 i = 0; i < o.size_; i++) {
		ar(o.At(i));
	}
}

template <class Archive, typename T>
void load(Archive& ar, Array<T>& o)
{
	i64 size;
	ar(size);
	o.Resize(size);
	for (i32 i = 0; i < size; i++) {
		ar(o.At(i));
	}
}

}