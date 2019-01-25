#include <catch2/catch.hpp>

#include "arduinostubs.hpp"

#include <memory>
#include <digitalinput.h>
#include <digitalknob.h>


SCENARIO("Digital Knob", "[DigitalKnob]") {
    //            std::cout << i << " ! " << dn->intern() << ":" << dn->presses() << "\n";

    DigitalKnob* dn = new DigitalKnob(1, false, 100);
    digitalReadStubbed = false;
    dn->init();
    dn->handle();

    GIVEN("A stubbed digital button.") {
        THEN("should all be false") {
            REQUIRE(dn->current() == false);
            REQUIRE(dn->isSingle() == false);
            REQUIRE(dn->isDouble() == false);
            REQUIRE(dn->isLong() == false);
        }
        THEN("Should correctly debounce") {
            DigitalKnob* db = new DigitalKnob(1, false, 150);
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

        for (int i = 0; i < 11; i++) {
            dn->handle();
        }

        digitalReadStubbed = true;

        for (int i = 0; i < 8; i++) {
            dn->handle();
        }

        digitalReadStubbed = false;
        dn->handle();

        THEN("should be be double clicked only") {
            REQUIRE(dn->current() == false);
            REQUIRE(dn->isSingle() == false);
            REQUIRE(dn->isDouble() == true);
            REQUIRE(dn->isDouble() == false); // Should reset itself
            REQUIRE(dn->isLong() == false);
        }
    }

    // Same pattern as double click with a little bit flip in the center
    GIVEN("we double click with error") {
        digitalReadStubbed = true;

        for (int i = 0; i < 8; i++) {
            dn->handle();
        }

        digitalReadStubbed = false;

        for (int i = 0; i < 6; i++) {
            dn->handle();
        }

        digitalReadStubbed = true;
        dn->handle();
        digitalReadStubbed = false;

        for (int i = 0; i < 7; i++) {
            dn->handle();
        }

        digitalReadStubbed = true;

        for (int i = 0; i < 9; i++) {
            dn->handle();
        }

        THEN("should be be double clicked only") {
            REQUIRE(dn->current() == true);
            REQUIRE(dn->isSingle() == false);
            REQUIRE(dn->isDouble() == false);
            REQUIRE(dn->isLong() == false);
        }
    }

    GIVEN("we single click") {
        digitalReadStubbed = true;

        for (int i = 0; i < 12; i++) {
            dn->handle();
        }

        digitalReadStubbed = false;

        for (int i = 0; i < 15; i++) {
            dn->handle();
        }


        THEN("should be be single clicked only") {
            REQUIRE(dn->current() == false);
            REQUIRE(dn->isSingle() == true);
            REQUIRE(dn->isSingle() == false); // Should reset itself
            REQUIRE(dn->isDouble() == false);
            REQUIRE(dn->isLong() == false);
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
            REQUIRE(dn->isLong() == false); // Should reset
            dn->handle();
            REQUIRE(dn->isLong() == true); // But should be back after the next handle
        }
        THEN("should be be long press only") {
            REQUIRE(dn->isLong() == true);
            digitalReadStubbed = false;
            dn->handle();
            REQUIRE(dn->isLong() == false); // Should reset at the first off bit

        }

    }

    GIVEN("we want to detect edges") {
        dn->resetButtons();
        dn->reset();

        THEN("should detect edge up") {
            digitalReadStubbed = false;
            dn->handle();
            digitalReadStubbed = true;
            dn->handle();
            REQUIRE(dn->isEdgeUp() == true); //
            REQUIRE(dn->isEdgeUp() == false); // Should reset
            dn->handle();
            REQUIRE(dn->isEdgeUp() == false); // Should stay reset
        }
        THEN("should detect edge down") {
            digitalReadStubbed = true;
            dn->handle();
            digitalReadStubbed = false;
            dn->handle();
            REQUIRE(dn->isEdgeDown() == true); //
            REQUIRE(dn->isEdgeDown() == false); // Should reset
            dn->handle();
            REQUIRE(dn->isEdgeDown() == false); // Should stay reset
        }
    }
}