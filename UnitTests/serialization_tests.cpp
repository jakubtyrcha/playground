#include "array.h"

#include "catch/catch.hpp"

#include "Array.Serialize.h"
#include <cereal/archives/json.hpp>

#include <sstream>

using namespace Playground;

TEST_CASE("arrays can be serialized and deserialized", "[array]")
{
	Array<i32> data;
	data.PushBack(1);
	data.PushBack(2);
	data.PushBack(3);
	data.PushBack(4);
	data.PushBack(5);
	data.PushBack(666);

	std::stringstream oos;
	{
		cereal::JSONOutputArchive archive(oos);
		archive(data);
	}

	{
		Array<i32> des;
		cereal::JSONInputArchive archive(oos);
		archive(des);

		REQUIRE(des.Size() == data.Size());
		for (i32 i = 0; i < des.Size(); i++) {
			REQUIRE(data[i] == des[i]);
		}
	}
}