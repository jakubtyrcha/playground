#include "Components.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("component can be added and removed", "[components]")
{
	// declare a component
	struct TestComponent {
		f32 x;
		f32 y;
		f32 z;
	};
	constexpr ComponentTypeId TestComponentTypeId = 0;
	using TestComponentId = ComponentId<TestComponentTypeId>;
	
	ComponentContainer<TestComponentId, DenseIndex<TestComponentId, Soa<TestComponent>>> test_components_;

	TestComponentId added = test_components_.Add();
	REQUIRE(test_components_.Size() == 1);
	
	test_components_.Remove(added);
	REQUIRE(test_components_.Size() == 0);
}

TEST_CASE("component can be accessed", "[components]")
{
	// declare a component
	struct TestComponent {
		f32 x;
		f32 y;
		f32 z;
	};
	constexpr ComponentTypeId TestComponentTypeId = 0;
	using TestComponentId = ComponentId<TestComponentTypeId>;
	
	DenseComponentArray<TestComponentId, TestComponent> test_components_;

	TestComponentId added = test_components_.Add();
	
	test_components_.AtMut<0>(added) = TestComponent{ .x = 1.f, .y = 2.f, .z = 3.f };

	REQUIRE(test_components_.DataSlice<0>()[0].x == 1.f);
	REQUIRE(test_components_.DataSlice<0>()[0].y == 2.f);
	REQUIRE(test_components_.DataSlice<0>()[0].z == 3.f);
}

TEST_CASE("can create soa component", "[components]")
{
	struct TestComponent_0 {
		f32 x;
		f32 y;
		f32 z;
	};

	struct TestComponent_1 {
		i32 f;
	};

	constexpr ComponentTypeId TestComponentTypeId = 0;
	using TestComponentId = ComponentId<TestComponentTypeId>;

	DenseComponentArray<TestComponentId, TestComponent_0, TestComponent_1> test_components_;

	TestComponentId added = test_components_.Add();
	

	test_components_.AtMut<0>(added) = TestComponent_0{ .x = 1.f, .y = 2.f, .z = 3.f };
	test_components_.AtMut<1>(added) = TestComponent_1{ .f = 7 };

	REQUIRE(test_components_.DataSlice<0>()[0].x == 1.f);
	REQUIRE(test_components_.DataSlice<0>()[0].y == 2.f);
	REQUIRE(test_components_.DataSlice<0>()[0].z == 3.f);
	REQUIRE(test_components_.DataSlice<1>()[0].f == 7);
}