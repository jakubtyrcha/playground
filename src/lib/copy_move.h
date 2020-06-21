#pragma once

template<typename T>
struct MoveableNonCopyable
{
	MoveableNonCopyable(const MoveableNonCopyable&) = delete;
	MoveableNonCopyable& operator = (const MoveableNonCopyable&) = delete;

	MoveableNonCopyable(MoveableNonCopyable&&) = default;
	MoveableNonCopyable& operator = (MoveableNonCopyable&&) = default;

protected:
	MoveableNonCopyable() = default;
	~MoveableNonCopyable() = default;
};
