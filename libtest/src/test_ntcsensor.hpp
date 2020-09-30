#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <NTCSensor.h>
#include <array>


TEST_CASE("Should Calculate steinHartValues", "[ntcsensor]") {
    float r, r1, r2, r3, t1, t2, t3, ka, kb, kc;
    r = 10000;
    r1 = 25415;
    t1 = 5;
    r2 = 10021;
    t2 = 25;
    r3 = 6545;
    t3 = 35;

    NTCSensor::calculateSteinhart(r, r1, t1, r2, t2, r3, t3, ka, kb, kc);

    REQUIRE(ka == Approx(0.00113836653));
    REQUIRE(kb == Approx(0.000232453211));
    REQUIRE(kc == Approx(0.0000000948887404));


    SECTION("Should measure temperature upstream") {
        NTCSensor sensor(0, true, 0.0f, 1.0f, r, ka, kb, kc);

        // https://ohmslawcalculator.com/voltage-divider-calculator
        analogReadStubbed = (1023 * r) / (r + r2);
        sensor.handle();
        REQUIRE(sensor.get() == Approx(24.91409f));

        analogReadStubbed = (1023 * r) / (r + r3);
        sensor.handle();
        REQUIRE(sensor.get() == Approx(34.96906));

        analogReadStubbed = (1023 * r) / (r + r1);
        sensor.handle();
        REQUIRE(sensor.get() == Approx(4.91589));
    }

    SECTION("Should measure temperature downstream") {
        NTCSensor sensor(0, false, 0.0f, 1.0f, r, ka, kb, kc);

        // https://ohmslawcalculator.com/voltage-divider-calculator
        analogReadStubbed = (1023 * r2) / (r + r2);
        sensor.handle();
        REQUIRE(sensor.get() == Approx(25.00327f));
    }
}