#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <analogin.h>
#include <numericinput.h>
#include <numericknob.h>

SCENARIO("Numeric Input", "[NumericInput]") {

    auto ai = std::make_shared<AnalogIn>(1.0f);
    NumericKnob ni(ai, 150.0f, 90.0f, 240.0f, 0.1);

    digitalReadPinStubbed = 0;
    analogReadStubbed = 512;
    ni.init();
    ai->init();
    // GIVEN("a situation where the button is not pressed") {
    //     digitalReadStubbed = 0;
    //     millisStubbed = 200;
    //     THEN("Input value should not change") {
    //         REQUIRE(ni.value() == Approx(150.0));
    //         ni.handle();
    //         REQUIRE(ni.value() == Approx(150.0));
    //     }
    // }
    GIVEN("a situation where the button is pressed") {
        digitalReadStubbed = 1;
        millisStubbed = millisStubbed + 200;
        THEN("Input should not change when analog does not change") {
            millisStubbed = millisStubbed + 200;
            REQUIRE(ni.value() == Approx(150.0));
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(150.0));
        }
        THEN("Input should change a little when analog changes a little") {
            millisStubbed = millisStubbed + 200;
            REQUIRE(ni.value() == 150.0);
            analogReadStubbed = analogReadStubbed + 5;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(150.1));
        }
        THEN("Input should change more when analog changes a more") {
            millisStubbed = millisStubbed + 200;
            REQUIRE(ni.value() == 150.0);
            analogReadStubbed = analogReadStubbed + 10;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(151.0));
        }
        THEN("Input should change a lot when analog changes a fast") {
            millisStubbed = millisStubbed + 200;
            REQUIRE(ni.value() == 150.0);
            analogReadStubbed = analogReadStubbed + 20;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(160.0));
        }
        THEN("Input should change back a lot when analog changes a fast") {
            millisStubbed = millisStubbed + 200;
            REQUIRE(ni.value() == 150.0);
            analogReadStubbed = analogReadStubbed - 20;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(140.0));
        }

        // THEN("Input should limit at minimum value") {
        //     millisStubbed = millisStubbed + 200;
        //     REQUIRE(ni.value() == 150.0);

        //     for (int i = 0; i < 10; i++) {
        //         millisStubbed = millisStubbed + 200;
        //         analogReadStubbed = analogReadStubbed - 20;
        //         ni.handle();
        //     }

        //     REQUIRE(ni.value() == Approx(90.0));
        // }

        // THEN("Input should limit at maximum value") {
        //     millisStubbed = millisStubbed + 200;
        //     REQUIRE(ni.value() == 150.0);

        //     for (int i = 0; i < 30; i++) {
        //         millisStubbed = millisStubbed + 200;
        //         analogReadStubbed = analogReadStubbed + 20;
        //         ni.handle();
        //     }

        //     REQUIRE(ni.value() == Approx(240.0));
        // }
    }
}

