#include "Hashmap.h"
#include "box.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch/catch.hpp"

using namespace Playground;

#include <unordered_map>

TEST_CASE("hashmap fill up", "hashmap_vs_unordered_map")
{
    BENCHMARK("Hashmap")
	{
		Hashmap<i32, i32> h;
        for(i32 i=0; i<100000; i++) {
            h.Insert(i, i);
        }
        return h.Size();
	};

    BENCHMARK("UnorderedMap")
	{
		std::unordered_map<i32, i32> h;
        for(i32 i=0; i<100000; i++) {
            h.insert(std::make_pair(i, i));
        }
        return h.size();
	};
}

TEST_CASE("hashmap query (hit)", "hashmap_vs_unordered_map")
{
    Hashmap<i32, i32> h;
    for (i32 i = 0; i < 100000; i++) {
        h.Insert(i, i);
    }

    BENCHMARK("Hashmap")
    {
        i32 sum = 0;
        for (i32 i = 0; i < 50000; i+=2) {
            sum += h.Contains(i) ? 1 : 0;
        }
        return sum;
    };

    std::unordered_map<i32, i32> um;
    for (i32 i = 0; i < 100000; i++) {
        um.insert(std::make_pair(i, i));
    }

    BENCHMARK("UnorderedMap")
    {
        i32 sum = 0;
        for (i32 i = 0; i < 50000; i+=2) {
            sum += As<i32>(um.count(i));
        }
        return sum;
    };
}

TEST_CASE("hashmap query (miss)", "hashmap_vs_unordered_map")
{
    Hashmap<i32, i32> h;
    for (i32 i = 0; i < 100000; i++) {
        h.Insert(i, i);
    }

    BENCHMARK("Hashmap")
    {
        i32 sum = 0;
        for (i32 i = 100000; i < 150000; i++) {
            sum += h.Contains(i) ? 1 : 0;
        }
        return sum;
    };

    std::unordered_map<i32, i32> um;
    for (i32 i = 0; i < 100000; i++) {
        um.insert(std::make_pair(i, i));
    }

    BENCHMARK("UnorderedMap")
    {
        i32 sum = 0;
        for (i32 i = 100000; i < 150000; i++) {
            sum += As<i32>(um.count(i));
        }
        return sum;
    };
}
