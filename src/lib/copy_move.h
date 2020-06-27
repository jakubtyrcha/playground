#pragma once

template<typename T>
T AlignedForward(T v, T a) {
	return (v + (a - 1)) & ~(a - 1);
}

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

template<typename T>
struct Pinned
{
	Pinned(const Pinned&) = delete;
	Pinned& operator = (const Pinned&) = delete;

	Pinned(Pinned&&) = delete;
	Pinned& operator = (Pinned&&) = delete;

protected:
	Pinned() = default;
	~Pinned() = default;
};
