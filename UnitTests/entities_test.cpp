#include "Entities.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("components can be attached and detached from entities", "[ecs]")
{
    Entities entities;

    EntityId e = entities.Spawn();

    using T0 = ComponentId<1>;
    using TMany = ComponentId<1000>;

    T0 c0 = T0::Make(0, 1);
    TMany c1 = TMany::Make(0, 1);

    entities.AttachComponent(e, c0);

    REQUIRE(entities.GetOwner(c0) == e);

    entities.AttachComponent(e, c1);

    REQUIRE(entities.GetOwner(c0) == e);
    REQUIRE(entities.GetOwner(c1) == e);

    for(i32 i=1; i<=100; i++) {
        entities.AttachComponent(e, TMany::Make(i, 1));
    }

    REQUIRE(entities.GetOwner(TMany::Make(100, 1)) == e);

    for(i32 i=1; i<=100; i+=2) {
        entities.DetachComponent(e, TMany::Make(i, 1));
    }

    for(i32 i=0; i<=100; i+=2) {
        REQUIRE(entities.GetOwner(TMany::Make(i, 1)) == e);    
    }

    for(i32 i = 100; i >= 80; i--) {
        TMany c = TMany::Make(i, 1);
        if(entities.GetOwner(c)) {
            entities.DetachComponent(e, c);
        }
    }

    for(i32 i = 20; i < 60; i++) {
        TMany c = TMany::Make(i, 1);
        if(entities.GetOwner(c)) {
            entities.DetachComponent(e, c);
        }
    }

    REQUIRE(entities.GetOwner(TMany::Make(78, 1)) == e);
    REQUIRE(entities.GetOwner(TMany::Make(76, 1)) == e);
    REQUIRE(entities.GetOwner(TMany::Make(60, 1)) == e);
}

TEST_CASE("components can be queried by type", "[ecs]")
{
    Entities entities;

    EntityId e0 = entities.Spawn();
    EntityId e1 = entities.Spawn();
    EntityId e2 = entities.Spawn();

    using T0 = ComponentId<1>;
    using TA = ComponentId<100>;
    using TB = ComponentId<101>;
    using TC = ComponentId<102>;
    using TD = ComponentId<103>;

    entities.AttachComponent(e0, T0::Make(0, 1));
    entities.AttachComponent(e1, T0::Make(1, 1));
    entities.AttachComponent(e2, T0::Make(2, 1));

    entities.AttachComponent(e0, TD::Make(1, 1));

    entities.AttachComponent(e1, TA::Make(1, 1));
    entities.AttachComponent(e1, TB::Make(1, 1));
    entities.AttachComponent(e1, TC::Make(1, 1));

    entities.AttachComponent(e2, TB::Make(2, 1));
    entities.AttachComponent(e2, TC::Make(2, 1));

    REQUIRE(entities.GetAnyComponentOfType(e0, T0::ComponentTypeId) == T0::Make(0, 1));

    REQUIRE(!entities.GetAnyComponentOfType(e0, TA::ComponentTypeId));

    REQUIRE(entities.GetAnyComponentOfType(e1, TA::ComponentTypeId) == TA::Make(1, 1));
    REQUIRE(entities.GetAnyComponentOfType(e1, TB::ComponentTypeId) == TB::Make(1, 1));
    REQUIRE(entities.GetAnyComponentOfType(e1, TC::ComponentTypeId) == TC::Make(1, 1));
    REQUIRE(!entities.GetAnyComponentOfType(e1, TD::ComponentTypeId));

    REQUIRE(!entities.GetAnyComponentOfType(e2, TA::ComponentTypeId));
    REQUIRE(entities.GetAnyComponentOfType(e2, TB::ComponentTypeId) == TB::Make(2, 1));
    REQUIRE(entities.GetAnyComponentOfType(e2, TC::ComponentTypeId) == TC::Make(2, 1));
    REQUIRE(!entities.GetAnyComponentOfType(e2, TD::ComponentTypeId));
}