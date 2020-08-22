#include "array.h"
#include "box.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch/catch.hpp"

using namespace Playground;

#include <vector>

TEST_CASE("fill up", "array_vs_vector")
{
    BENCHMARK("Array")
	{
		Array<int> big_array;
        for(i32 i=0; i<100000; i++) {
            big_array.PushBack(i);
        }
	};

    BENCHMARK("Vector")
	{
		std::vector<int> big_vec;
        for(i32 i=0; i<100000; i++) {
            big_vec.push_back(i);
        }
	};
}

TEST_CASE("access via iterator", "array_vs_vector")
{
    Array<int> big_array;
    for (i32 i = 0; i < 100000; i++) {
        big_array.PushBack(i);
    }

    std::vector<int> big_vec;
    for (i32 i = 0; i < 100000; i++) {
        big_vec.push_back(i);
    }

    BENCHMARK("Array")
    {
        i32 sum = 0;
        for (i32 i : big_array) {
            sum += i;
        }
        return sum;
    };

    BENCHMARK("Vector")
    {
        i32 sum = 0;
        for (i32 i : big_vec) {
            sum += i;
        }
        return sum;
    };
}

TEST_CASE("access via index", "array_vs_vector")
{
    Array<int> big_array;
    for (i32 i = 0; i < 100000; i++) {
        big_array.PushBack(i);
    }

    std::vector<int> big_vec;
    for (i32 i = 0; i < 100000; i++) {
        big_vec.push_back(i);
    }

    BENCHMARK("Array")
    {
        i32 sum = 0;
        for (i32 i = 0, N = As<i32>(big_array.Size()); i< N; i++) {
            sum += big_array[i];
        }
        return sum;
    };

    BENCHMARK("Vector")
    {
        i32 sum = 0;
        for (i32 i = 0, N = As<i32>(big_vec.size()); i< N; i++) {
            sum += big_vec[i];
        }
        return sum;
    };
}