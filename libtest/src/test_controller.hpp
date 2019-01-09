#include <catch2/catch.hpp>

#include <memory>

#include "mocks.hpp"
#include "arduinostubs.hpp"
#include <bbqfanonly.h>
#include <iomanip>

// Run with make; ./tests [GRAPH] > out.csv
// Plot with https://plot.ly/create/#/
TEST_CASE("Graph Controller against simulated oven", "[GRAPH]") {
    MockedOven oven;
    MockedTemperature* mockedTemp = new MockedTemperature(oven.temperature());
    std::shared_ptr<TemperatureSensor> uMockedTemp(mockedTemp);
    MockedFan* mockedFan = new MockedFan();
    std::shared_ptr<MockedFan> uMockedFan(mockedFan);
    BBQFanOnly* bbqFanOnly = new BBQFanOnly(std::move(uMockedTemp), std::move(uMockedFan));
    bbqFanOnly->init();
    bbqFanOnly->setPoint(25);
    mockedTemp->set(oven.temperature()); // set temperature sensor to oven temperature

    std::cout << "i,Temperature,setPoint,fan,deltaError,lastError,drop\n";

    for (int i = 0; i < 12000000; i = i + 100) {
        millisStubbed = i;
        oven.airFlow(mockedFan->speed()); // Set airflow of oven to fanspeed
        mockedTemp->set(oven.temperature()); // set temperature sensor to oven temperature
        oven.handle();
        bbqFanOnly->handle();

        if (i % 5000 == 0 && i > 0) {
            std::cout
                    << i << ","
                    << oven.temperature() << ","
                    << bbqFanOnly->setPoint() << ","
                    << mockedFan->speed() << ","
                    << bbqFanOnly->deltaErrorInput() << ","
                    << bbqFanOnly->lastErrorInput() << ","
                    << bbqFanOnly->tempDropFilteredInput() << "\n";
        }
    }
}

TEST_CASE("Should convert char values to other types", "[propertyvalue][ALL]") {
    MockedOven oven;
    MockedTemperature* mockedTemp = new MockedTemperature(oven.temperature());
    std::shared_ptr<TemperatureSensor> uMockedTemp(mockedTemp);
    MockedFan* mockedFan = new MockedFan();
    std::shared_ptr<MockedFan> uMockedFan(mockedFan);
    BBQFanOnly* bbqFanOnly = new BBQFanOnly(std::move(uMockedTemp), std::move(uMockedFan));
    bbqFanOnly->setPoint(130);
    mockedTemp->set(oven.temperature()); // set temperature sensor to oven temperature

    for (int i = 0; i < 6000000; i = i + 100) {
        millisStubbed = i;
        oven.airFlow(mockedFan->speed()); // Set airflow of oven to fanspeed
        mockedTemp->set(oven.temperature()); // set temperature sensor to oven temperature
        oven.handle();
        bbqFanOnly->handle();
        //std::cout << std::fixed << std::setw(11) << std::setprecision(2) << ((float)i / 1000) << " temp:" << oven.temperature() << " fan speed:" << mockedFan->speed() << " drop30:" << bbqFanOnly->temperatureDrop() << " lid:" << bbqFanOnly->lidOpen() << " lowCharcoal:" <<  bbqFanOnly->lowCharcoal() << "\n";
        //        if (oven.temperature()>120.0) {
        //          oven.lidOpen(true);
        //     }
    }
}


TEST_CASE("Test Oven", "[oven][.]") {
    MockedOven oven;
    int fanSpeed = 100;

    for (int i = 0; i < 6000000; i = i + 100) {
        oven.airFlow(fanSpeed); // Set airflow of oven to fanspeed

        if (oven.temperature() > 130 && fanSpeed > 0) {
            fanSpeed = 0;
        }

        if (oven.temperature() < 70 && fanSpeed == 0) {
            fanSpeed = 100;
        }

        oven.handle();
        // std::cout << ((float)i/1000) << " temp:" << oven.temperature() << " fan speed:" << fanSpeed << "\n";
    }
}

TEST_CASE("BBQ Fan Only Fussy Set Creation", "[fuzzySet]") {

    BBQFanOnlyConfig config;
    SECTION("With 2 items") {
        std::array<float, 2> arr1 = {0, 2.5};
        FuzzySet* fs = BBQFanOnly::fuzzyFromVector(arr1, false);
        REQUIRE(fs->getPointA() == Approx(-2.5));
        REQUIRE(fs->getPointB() == Approx(0.0));
        REQUIRE(fs->getPointC() == Approx(0.0));
        REQUIRE(fs->getPointD() == Approx(2.5));
    }
    SECTION("With 2 items flipped") {
        std::array<float, 2> arr1 = {0, 2.5};
        FuzzySet* fs = BBQFanOnly::fuzzyFromVector(arr1, true);
        REQUIRE(fs->getPointA() == Approx(-2.5));
        REQUIRE(fs->getPointB() == Approx(0.0));
        REQUIRE(fs->getPointC() == Approx(0.0));
        REQUIRE(fs->getPointD() == Approx(2.5));
    }

    SECTION("With 4 items") {
        std::array<float, 4> arr1 = {100, 800, 900, 1000};
        FuzzySet* fs = BBQFanOnly::fuzzyFromVector(arr1, false);
        REQUIRE(fs->getPointA() == Approx(100.0));
        REQUIRE(fs->getPointB() == Approx(800.0));
        REQUIRE(fs->getPointC() == Approx(900.0));
        REQUIRE(fs->getPointD() == Approx(1000.0));
    }
    SECTION("With 4 items flipped") {
        std::array<float, 4> arr1 = {100, 800, 900, 1000};
        FuzzySet* fs = BBQFanOnly::fuzzyFromVector(arr1, true);
        REQUIRE(fs->getPointA() == Approx(-1000.0));
        REQUIRE(fs->getPointB() == Approx(-900.0));
        REQUIRE(fs->getPointC() == Approx(-800.0));
        REQUIRE(fs->getPointD() == Approx(-100.0));
    }

}

