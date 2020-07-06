#pragma once

#include "containers_shared.h"
#include "array.h"
#include "bitarray.h"
#include "hash.h"
#include "algorithms.h"

// heavily inspired by https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// TODO: this is a bit messy, make sure all cominations of trivially copyable and movable keys and values work

namespace Containers
{
	template<typename T>
	constexpr bool HashKeysByValue()
	{
		return (sizeof(T) <= 32) && std::is_trivial_v<T>;
	}

	i64 GetHashmapSize(i64 slots);
	i64 HashmapSlot(u64 hash, i64 mod);

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
			const Hashmap* hashmap_;
			i64 index_ = 0;

			Iterator(const Hashmap* hashmap, i64 index) : hashmap_(hashmap) {
				// TODO: should iterator move this, or the place constructing it?
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

			K& Key() const {
				return hashmap_->Key(index_);
			}

			V& Value() const {
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

			struct KeyValue {
				K& key;
				V& value;
			};

			KeyValue operator*() const {
				return { .key = Key(), .value = Value() };
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

		i64 Capacity() const {
			return capacity_;
		}

		void Clear()
		{
			if (keys_) {
				for (auto iter = begin(); iter != end(); ++iter) {
					if constexpr (!std::is_trivial_v<K>) {
						(keys_ + iter.index_)->K::~K();
					}
					if constexpr (!std::is_trivial_v<V>) {
						(values_ + iter.index_)->V::~V();
					}
				}
				//static_assert(std::is_trivial_v<K>);
				//static_assert(std::is_trivial_v<V>);
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
			return Algorithms::min(static_cast<i64>(FastLog2(static_cast<u32>(capacity_))), capacity_ - 1);
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

				if constexpr (std::is_move_assignable_v<V>) {
					for(i64 i=0; i<new_capacity; i++) {
						new (values_ + i) V{};
					}
				}

				slot_states_.Resize(new_capacity);
				capacity_ = new_capacity;
				return;
			}

			Hashmap<K, V> rehashed;
			rehashed.ReserveForCapacity(new_capacity);

			for (auto iter = begin(); iter != end(); ++iter) {
				if constexpr(std::is_trivially_copyable_v<V>) {
					bool i = rehashed.Insert(iter.Key(), iter.Value());
					DEBUG_ASSERT(i, containers_module{});
				}
				else {
					bool i = rehashed.InsertRvalueRef(iter.Key(), std::move(iter.Value()));
					DEBUG_ASSERT(i, containers_module{});
				}
			}

			*this = std::move(rehashed);
		}

		void Reserve(i64 size)
		{
			i64 new_capacity = GetHashmapSize(MinCapacity(size));

			ReserveForCapacity(new_capacity);
		}

		template<typename = std::enable_if<std::is_trivially_copyable_v<V>>::type>
		bool Insert(K key, V value)
		{
			static_assert(std::is_trivially_copyable_v<K>);

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

		template<typename = std::enable_if<std::is_move_assignable_v<V>>::type>
		bool InsertRvalueRef(K key, V && value)
		{
			static_assert(std::is_trivially_copyable_v<K>);

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
					values_[index] = std::move(value);
					inserted = true;
					return false;
				}

				if (slot_states_.GetBit(index) == false) {
					slot_states_.SetBit(index, true);
					keys_[index] = key;
					values_[index] = std::move(value);
					size_++;
					inserted = true;
					break;
				}

				index++;
			}

			if (!inserted) {
				ReserveForCapacity(GetHashmapSize(capacity_ + 1));

				return InsertRvalueRef(key, std::move(value));
			}

			return true;
		}

		void _AdjustProbing(i64 deleted_index) {
			i64 max_probe_count = MaxProbeCount();

			for (i64 offset = 1; offset < max_probe_count; offset++) {
				i64 index = deleted_index + offset;
				if (index >= capacity_) {
					index -= capacity_;
				}

				if (slot_states_.GetBit(index) == false) {
					break;
				}

				u64 hash = Hash::HashValue(keys_[index]);
				i64 preferred_index = HashmapSlot(hash, capacity_);

				if (preferred_index == index) {
					continue;
				}

				i64 distance_if_moved = deleted_index - preferred_index;
				if (distance_if_moved < 0) {
					distance_if_moved += capacity_;
				}
				i64 current_distance = index - preferred_index;
				if (current_distance < 0) {
					current_distance += capacity_;
				}

				if (distance_if_moved > current_distance) {
					continue;
				}

				keys_[deleted_index] = keys_[index];
				values_[deleted_index] = values_[index];
				slot_states_.SetBit(deleted_index, true);
				slot_states_.SetBit(index, false);

				_AdjustProbing(index);
			}
		}

		Iterator Remove(K key) {
			i64 index = *_FindIndex(key);

			static_assert(std::is_trivial_v<K>);
			static_assert(std::is_trivial_v<V>);

			slot_states_.SetBit(index, false);
			size_--;

			_AdjustProbing(index);

			return Iterator{ this, index };
		}

		K& Key(i64 index) const {
			//static_assert(std::is_trivially_copyable_v<K>);
			DEBUG_ASSERT(slot_states_.GetBit(index), containers_module{});
			return keys_[index];
		}

		V& Value(i64 index) const {
			//static_assert(std::is_trivially_copyable_v<V>);
			DEBUG_ASSERT(slot_states_.GetBit(index), containers_module{});
			return values_[index];
		}

		Core::Optional<i64> _FindIndex(K key) const {
			if(capacity_ == 0 || size_ == 0) {
				return {};
			}

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

			return {};
		}

		bool Contains(K key) const
		{
			return static_cast<bool>(_FindIndex(key));
		}

		V At(K key) const
		{
			static_assert(std::is_trivially_copyable_v<V>);
			i64 index = *_FindIndex(key);
			return values_[index];
		}

		Core::Optional<V*> Find(K key) const {
			//static_assert(std::is_trivially_copyable_v<V>);
			Core::Optional<i64> maybe_index = _FindIndex(key);

			if(!maybe_index) {
				return {};
			}

			return &values_[*maybe_index];
		}

		i64 Size() const
		{
			return size_;
		}
	};
}
