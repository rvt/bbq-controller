#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <onoffventilator.h>
#include <array>


TEST_CASE("Should start on/off ventilator", "[utils]") {
    millisStubbed = 0;
    SECTION("Should be 0") {
        OnOffVentilator fan(0, 1000);
        fan.speed(0);
        REQUIRE(digitalWriteStubbed == 0);
        millisStubbed=999;
        fan.speed(0);
        REQUIRE(digitalWriteStubbed == 0);
    }
    SECTION("Should be 50") {
        OnOffVentilator fan(0, 1000);
        fan.speed(50);
        REQUIRE(digitalWriteStubbed == 1);
        millisStubbed=499;
        fan.speed(50);
        REQUIRE(digitalWriteStubbed == 1);
        millisStubbed=501;
        fan.speed(50);
        REQUIRE(digitalWriteStubbed == 0);
    }
    SECTION("Should be 75") {
        OnOffVentilator fan(0, 1000);
        fan.speed(75);
        REQUIRE(digitalWriteStubbed == 1);
        millisStubbed=499;
        fan.speed(75);
        REQUIRE(digitalWriteStubbed == 1);
        millisStubbed=751;
        fan.speed(75);
        REQUIRE(digitalWriteStubbed == 0);
    }

    SECTION("should be off at 0.9 speed with pwm offset") {
        OnOffVentilator fan(0, 40);
        fan.speed(0.9);
        REQUIRE(digitalWriteStubbed == 0);
    }

}

