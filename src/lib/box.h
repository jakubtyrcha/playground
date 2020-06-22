#pragma once

#include <magnum/CorradePointer.h>

template<typename T> using Box = Corrade::Containers::Pointer<T>;

template<typename T, typename ...Args> Box<T> MakeBox(Args&&... args) {
	return Box<T>{new T{ std::forward<Args>(args)... }};
}