#include <catch2/catch.hpp>

#include <memory>
#include <optparser.hpp>
#include <utils.h>
#include <array>


TEST_CASE("Should beable to load vector from config", "[utils]") {

    SECTION("Should get values with equal data set size") {
        const char* inputDataSet = "k1=11,22,33";
        std::array<float, 3> currentValues = {1, 2, 3};
        std::array<float, 3>  newValues = getConfigArray("k1", "k1", inputDataSet, currentValues);
        REQUIRE(newValues[0] == Approx(11.0));
        REQUIRE(newValues[1] == Approx(22.0));
        REQUIRE(newValues[2] == Approx(33.0));
    }
    SECTION("Should get old data with un equal dataset (to much)") {
        const char* inputDataSet = "k2=34,35,36,37";
        std::array<float, 3> currentValues = {1, 2, 3};
        std::array<float, 3> newValues = getConfigArray("k1", "k1", inputDataSet, currentValues);
        REQUIRE(newValues[0] == Approx(34.0));
        REQUIRE(newValues[1] == Approx(35.0));
        REQUIRE(newValues[2] == Approx(36.0));
    }
    SECTION("Should get old data with un equal dataset (to little)") {
        const char* inputDataSet = "k2=34,35";
        std::array<float, 3> currentValues = {1, 2, 3};
        auto newValues = getConfigArray("k1", "k1", inputDataSet, currentValues);
        REQUIRE(newValues[0] == Approx(1.0));
        REQUIRE(newValues[1] == Approx(2.0));
        REQUIRE(newValues[2] == Approx(3.0));
    }

    SECTION("Should beable to split string") {
        splitString<float, 4>("1.2,5,8,6,-12,6.79", ',', [&](const std::string & value) {
            return std::stof(value);
        }
                             );
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

