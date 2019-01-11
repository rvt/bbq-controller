#include <catch2/catch.hpp>

#include <memory>
#include <optparser.h>
#include <utils.h>
#include <array>


TEST_CASE("Should bale to load vector from config", "[utils]") {

    SECTION("Should get values with equal data set size") {
        const char* inputDataSet = "k1=11,22,33";
        std::vector<float> currentValues = {1, 2, 3};
        auto newValues = getConfigVector("k1", inputDataSet, currentValues);
        REQUIRE(newValues[0] == Approx(11.0));
        REQUIRE(newValues[1] == Approx(22.0));
        REQUIRE(newValues[2] == Approx(33.0));
    }
    SECTION("Should get old data with un equal dataset") {
        const char* inputDataSet = "k2=34,35,36,37";
        std::vector<float> currentValues = {1, 2, 3};
        auto newValues = getConfigVector("k1", inputDataSet, currentValues);
        REQUIRE(newValues[0] == Approx(1.0));
        REQUIRE(newValues[1] == Approx(2.0));
        REQUIRE(newValues[2] == Approx(3.0));
    }



}


SCENARIO("Should override speed", "[ventilator]") {
    GIVEN("A default mocked ventilator") {

        MockedFan* mockedFan = new MockedFan();
        mockedFan->speedOverride(-1);
        mockedFan->speed(50);

        WHEN("When speed set to 24") {
            mockedFan->speed(24);
            THEN("speedOverride should be -1 and speed 24") {
                REQUIRE(mockedFan->speed() == Approx(24));
                REQUIRE(mockedFan->speedOverride() == Approx(-1));
            }
        }

        WHEN("When change speedOverride set > 0") {
            mockedFan->speedOverride(38);
            THEN("speed should change") {
                REQUIRE(mockedFan->speed() == Approx(38));
                REQUIRE(mockedFan->speedOverride() == Approx(38));
            }
        }

        WHEN("When change speedOverride set to -1") {
            mockedFan->speedOverride(60);
            mockedFan->speedOverride(-1);
            THEN("speed should change back to origional") {
                REQUIRE(mockedFan->speedOverride() == Approx(-1));
                REQUIRE(mockedFan->speed() == Approx(50));
            }
        }
    }
}

