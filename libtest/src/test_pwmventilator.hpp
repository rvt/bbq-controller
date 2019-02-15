#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <pwmventilator.h>
#include <array>


TEST_CASE("Should start pwm ventilator", "[utils]") {

    SECTION("Should have range 0..100") {
        PWMVentilator fan(0,0);
        fan.speed(0);
        REQUIRE(analogWriteStubbed == 0);
        fan.speed(50);
        REQUIRE(analogWriteStubbed == 256);
        fan.speed(100);
        REQUIRE(analogWriteStubbed == 512);
    }

    SECTION("should be off at 0.9 speed with pwm offset") {
        PWMVentilator fan(0,40);
        fan.speed(0.9);
        REQUIRE(analogWriteStubbed == 0);
    }

     SECTION("Should support start pwm") {
        PWMVentilator fan(0,40);
        fan.speed(1);
        REQUIRE(analogWriteStubbed == 207);
        fan.speed(50);
        REQUIRE(analogWriteStubbed == 358);
        fan.speed(100);
        REQUIRE(analogWriteStubbed == 512);
    }

}

