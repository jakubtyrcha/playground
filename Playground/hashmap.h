#pragma once

#include "containers_shared.h"
#include "array.h"
#include "bitarray.h"
#include "hash.h"

namespace Containers
{
	template<typename T>
	constexpr bool HashKeysByValue()
	{
		return (sizeof(T) < 16) && std::is_trivial_v<T>;
	}

	i64 GetHashmapSize(i64 slots);
	i64 HashmapSlot(u64 hash, i64 mod);

	template<typename T>
	T min(T l, T r) {
		if (r < l) {
			return r;
		}
		return l;
	}

	u32 FastLog2(u32 v);

	template<typename K, typename V>
	struct Hashmap
	{
		K* keys_ = nullptr;
		V* values_ = nullptr;
		Bitarray slot_states_;
		i64 size_ = 0;
		i64 capacity_ = 0;

		struct Iterator
		{
			const Hashmap* const hashmap_;
			i64 index_ = 0;

			Iterator(const Hashmap* const hashmap, i64 index) : hashmap_(hashmap) {
				if (index >= hashmap_->capacity_) {
					index_ = hashmap_->capacity_;
				}
				else {
					index_ = hashmap_->slot_states_.GetNextBitSet(index);
				}
			}

			Iterator operator++(int) {
				if (index_ + 1 == hashmap_->capacity_) {
					index_ = index_ + 1;
				}
				else {
					index_ = hashmap_->slot_states_.GetNextBitSet(index_ + 1);
				}
				return *this;
			}

			Iterator& operator++() {
				if (index_ + 1 == hashmap_->capacity_) {
					index_ = index_ + 1;
				}
				else {
					index_ = hashmap_->slot_states_.GetNextBitSet(index_ + 1);
				}
				return *this;
			}

			K Key() const {
				return hashmap_->Key(index_);
			}

			V Value() const {
				return hashmap_->Value(index_);
			}

			bool operator ==(Iterator other) const
			{
				return hashmap_ == other.hashmap_ && index_ == other.index_;
			}

			bool operator !=(Iterator other) const
			{
				return !((*this) == other);
			}
		};

		Hashmap()
		{
			static_assert(HashKeysByValue<K>());
		}

		~Hashmap()
		{
		}

		Hashmap(Hashmap&& other)
		{
			keys_ = other.keys_;
			values_ = other.values_;
			slot_states_ = std::move(other.slot_states_);
			size_ = other.size_;
			capacity_ = other.capacity_;

			other.keys_ = nullptr;
			other.values_ = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;
		}

		Hashmap& operator=(Hashmap&& other)
		{
			Clear();

			keys_ = other.keys_;
			values_ = other.values_;
			slot_states_ = std::move(other.slot_states_);
			size_ = other.size_;
			capacity_ = other.capacity_;

			other.keys_ = nullptr;
			other.values_ = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;

			return *this;
		}

		void Clear()
		{
			if (keys_) {
				static_assert(std::is_trivial_v<K>);
				static_assert(std::is_trivial_v<V>);
				free(keys_);
				free(values_);
			}

			keys_ = nullptr;
			values_ = nullptr;
			slot_states_.Resize(0);
			slot_states_.Shrink();
			size_ = 0;
			capacity_ = 0;
		}

		i64 MaxProbeCount() const {
			return min(static_cast<i64>(FastLog2(static_cast<u32>(capacity_))), capacity_ - 1);
		}

		Iterator begin() const {
			return Iterator(this, 0);
		}

		Iterator end() const {
			return Iterator(this, capacity_);
		}

		i64 MinCapacity(i64 size)
		{
			return size * 2 + 1;
		}

		void ReserveForCapacity(i64 new_capacity)
		{
			if (new_capacity <= capacity_) {
				return;
			}

			if (size_ == 0)
			{
				keys_ = static_cast<K*>(realloc(keys_, sizeof(K) * new_capacity));
				values_ = static_cast<V*>(realloc(values_, sizeof(V) * new_capacity));
				slot_states_.Resize(new_capacity);
				capacity_ = new_capacity;
				return;
			}

			Hashmap<K, V> rehashed;
			rehashed.ReserveForCapacity(new_capacity);

			//for (auto iter : (*this)) { // TODO: fix range for, why doesn't it compile?
			for (auto iter = begin(); iter != end(); ++iter) {
				bool i = rehashed.Insert(iter.Key(), iter.Value());
				DEBUG_ASSERT(i, containers_module{});
			}

			*this = std::move(rehashed);
		}

		void Reserve(i64 size)
		{
			i64 new_capacity = GetHashmapSize(MinCapacity(size));

			ReserveForCapacity(new_capacity);
		}

		bool Insert(K key, V value)
		{
			static_assert(std::is_trivially_copyable_v<K>);
			static_assert(std::is_trivially_copyable_v<V>);

			if (capacity_ < MinCapacity(size_ + 1)) {
				Reserve(size_ + 1);
			}

			u64 hash = Hash::HashValue(key);
			i64 index = HashmapSlot(hash, capacity_);

			i64 max_probe_count = MaxProbeCount();

			bool inserted = false;

			for (i64 offset = 0; offset < max_probe_count; offset++) {
				if (index == capacity_) {
					index = 0;
				}

				if (slot_states_.GetBit(index) == true && keys_[index] == key) {
					values_[index] = value;
					inserted = true;
					return false;
				}

				if (slot_states_.GetBit(index) == false) {
					slot_states_.SetBit(index, true);
					keys_[index] = key;
					values_[index] = value;
					size_++;
					inserted = true;
					break;
				}

				index++;
			}

			if (!inserted) {
				ReserveForCapacity(GetHashmapSize(capacity_ + 1));

				return Insert(key, value);
			}

			return true;
		}

		void Remove(K key)
		{
			i64 index = FindIndex(key);
			DEBUG_ASSERT(index != -1, containers_module{});

			static_assert(std::is_trivial_v<K>);
			static_assert(std::is_trivial_v<V>);

			slot_states_.SetBit(index, false);
			size_--;

			while (true)
			{
				i64 next = index + 1;
				if (next == capacity_) {
					next = 0;
				}

				if (slot_states_.GetBit(next) == false) {
					break;
				}

				u64 hash = Hash::HashValue(keys_[next]);
				i64 preferred_index = HashmapSlot(hash, capacity_);

				if (preferred_index != next) {
					keys_[index] = keys_[next];
					values_[index] = values_[next];
					slot_states_.SetBit(index, true);
					slot_states_.SetBit(next, false);

					index = next;
				}
				else {
					break;
				}
			}
		}

		K Key(i64 index) const {
			static_assert(std::is_trivially_copyable_v<K>);
			DEBUG_ASSERT(slot_states_.GetBit(index), containers_module{});
			return keys_[index];
		}

		V Value(i64 index) const {
			static_assert(std::is_trivially_copyable_v<V>);
			DEBUG_ASSERT(slot_states_.GetBit(index), containers_module{});
			return values_[index];
		}

		i64 FindIndex(K key) const {
			u64 hash = Hash::HashValue(key);
			i64 index = HashmapSlot(hash, capacity_);
			i64 max_probe_count = MaxProbeCount();

			for (i64 offset = 0; offset < max_probe_count; offset++) {
				if (index == capacity_) {
					index = 0;
				}

				if (slot_states_.GetBit(index) && keys_[index] == key) {
					return index;
				}

				index = index + 1;
			}

			return -1;
		}

		bool Contains(K key) const
		{
			return FindIndex(key) != -1;
		}

		V At(K key) const
		{
			static_assert(std::is_trivially_copyable_v<V>);
			i64 index = FindIndex(key);
			DEBUG_ASSERT(index != -1, containers_module{});
			return values_[index];
		}

		i64 Size() const
		{
			return size_;
		}
	};
}
