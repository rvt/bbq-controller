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

