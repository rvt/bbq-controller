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
    ai->init();
    GIVEN("a analog input and numeric knob") {
        digitalReadStubbed = 1;
        THEN("Input should not change when analog does not change") {
            REQUIRE(ni.value() == Approx(150.0));
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(150.0));
        }
        THEN("Input should change a little when analog changes a little") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = analogReadStubbed + 5;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(150.5));
        }
        THEN("Input should change more when analog changes a more") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = analogReadStubbed + 10;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(151.0));
        }
        THEN("Input should change a lot when analog changes a fast") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = analogReadStubbed + 20;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(152.0));
        }
        THEN("Input should change back a lot when analog changes a fast") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = analogReadStubbed - 20;
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(148.0));
        }

        THEN("Input should limit at minimum value") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = 1024;
            ai->handle();

            while (analogReadStubbed > 100) {
                analogReadStubbed = analogReadStubbed - 100;
                ai->handle();
                ni.handle();
            }

            REQUIRE(ni.value() == Approx(90.0));
        }

        THEN("Input should limit at maximum value") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = 0;
            ai->handle();

            while (analogReadStubbed < 1024 - 100) {
                analogReadStubbed = analogReadStubbed + 100;
                ai->handle();
                ni.handle();
            }

            REQUIRE(ni.value() == Approx(240.0));
        }

        THEN("Should beable to set value") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = 0;
            ai->handle();
            ni.handle();
            ni.value(180.0);
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(180.0));
        }
        THEN("Should beable to set value not higher or lower than min and max") {
            REQUIRE(ni.value() == Approx(150.0));
            analogReadStubbed = 0;
            ai->handle();
            ni.handle();
            ni.value(0.0);
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(90.0));
            ai->handle();
            ni.handle();
            ni.value(300.0);
            ai->handle();
            ni.handle();
            REQUIRE(ni.value() == Approx(240.0));
        }
    }
}

