#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <digitalinput.h>
#include <digitalknob.h>


SCENARIO("Digital Knob", "[DigitalKnob]") {
    //            std::cout << i << " ! " << dn->intern() << ":" << dn->presses() << "\n";

    DigitalKnob* dn = new DigitalKnob(1, 100);
    digitalReadStubbed = false;
    dn->init();
    dn->handle();

    GIVEN("at time zero.") {
        THEN("should all be false") {
            REQUIRE(dn->current() == false);
            REQUIRE(dn->isSingle() == false);
            REQUIRE(dn->isDouble() == false);
            REQUIRE(dn->isLong() == false);
        }
        THEN("Should correctly debounce") {
            DigitalKnob* db = new DigitalKnob(1, 150);
            digitalReadStubbed = true;
            dn->handle();
            REQUIRE(db->current() == false);
            digitalReadStubbed = false;
            dn->handle();
            REQUIRE(db->current() == false);
            dn->handle();
            REQUIRE(db->current() == false);
            dn->handle();
            REQUIRE(db->current() == false);
            delete db;
        }
    }

    GIVEN("we double click") {
        digitalReadStubbed = true;

        for (int i = 0; i < 8; i++) {
            dn->handle();
        }

        digitalReadStubbed = false;

        for (int i = 0; i < 14; i++) {
            dn->handle();
        }

        digitalReadStubbed = true;

        for (int i = 0; i < 9; i++) {
            dn->handle();
        }


        THEN("should be be double clicked only") {
            REQUIRE(dn->current() == true);
            REQUIRE(dn->isSingle() == false);
            REQUIRE(dn->isDouble() == true);
            REQUIRE(dn->isLong() == false);
            dn->resetButtons();
            REQUIRE(dn->isSingle() == false);
        }
    }

    GIVEN("we single click") {
        digitalReadStubbed = true;

        for (int i = 0; i < 8; i++) {
            dn->handle();
        }

        digitalReadStubbed = false;

        for (int i = 0; i < 18; i++) {
            dn->handle();
        }


        THEN("should be be single clicked only") {
            REQUIRE(dn->current() == false);
            REQUIRE(dn->isSingle() == true);
            REQUIRE(dn->isDouble() == false);
            REQUIRE(dn->isLong() == false);
            dn->resetButtons();
            REQUIRE(dn->isSingle() == false);
        }
    }

    GIVEN("we long press") {
        dn->resetButtons();
        digitalReadStubbed = true;

        for (int i = 0; i < 50; i++) {
            dn->handle();
        }

        THEN("should be be long press only") {
            REQUIRE(dn->current() == true);
            REQUIRE(dn->isSingle() == false);
            REQUIRE(dn->isDouble() == false);
            REQUIRE(dn->isLong() == true);

            dn->resetButtons();
            REQUIRE(dn->isLong() == false);
            REQUIRE(dn->current() == false);
        }

    }
}