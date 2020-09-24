#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <pwmventilator.h>
#include <array>


TEST_CASE("Should start pwm ventilator", "[utils]") {

    SECTION("Should have linear range 0..100") {
        millisStubbed = 0;
        PWMVentilator fan(0, 0, 0);
        fan.speed(0);
        REQUIRE(analogWriteStubbed == 0);
        fan.speed(1);
        millisStubbed = 1000;
        fan.speed(1);
        REQUIRE(analogWriteStubbed == 3);
        fan.speed(50);
        REQUIRE(analogWriteStubbed == 128);
        fan.speed(100);
        REQUIRE(analogWriteStubbed == 255);
    }

    SECTION("Should have range 50..100") {
        millisStubbed = 0;
        PWMVentilator fan(0, 50, 0);
        fan.speed(0);
        REQUIRE(analogWriteStubbed == 0);
        fan.speed(1);
        millisStubbed = 1000;
        fan.speed(50);
        REQUIRE(analogWriteStubbed == 191);
        fan.speed(100);
        REQUIRE(analogWriteStubbed == 255);
    }

    SECTION("should be off at 0.9 speed with pwm offset") {
        millisStubbed = 0;
        PWMVentilator fan(0, 40, 0);
        fan.speed(10);
        millisStubbed = 1000;
        fan.speed(0.9);
        REQUIRE(analogWriteStubbed == 0);
    }

    SECTION("Should support start pwm") {
        millisStubbed = 0;
        PWMVentilator fan(0, 0, 0);
        fan.speed(10);
        REQUIRE(analogWriteStubbed == 191);
        millisStubbed = 1000;
        fan.speed(10);
        REQUIRE(analogWriteStubbed == 26);
    }

}

