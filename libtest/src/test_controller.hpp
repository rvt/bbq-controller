#include <catch2/catch.hpp>

#include <memory>

#include "mocks.hpp"
#include "arduinostubs.hpp"
#include <bbqfanonly.h>
#include <iomanip>

TEST_CASE("Graph Controller against simulated oven FuzzySet", "[GRAPH][.]") {
    FuzzySet* tdf = new FuzzySet(-2000, -1000, -1000, -500);
    tdf->calculatePertinence(-10);
}

// Run with :  rm tests; make; ./tests [GRAPH] > out.csv
// Plot with https://plot.ly/create/#/
TEST_CASE("Graph Controller against simulated oven", "[GRAPH][.]") {
    MockedOven oven;
    MockedTemperature* mockedTemp = new MockedTemperature(oven.temperature());
    std::shared_ptr<TemperatureSensor> uMockedTemp(mockedTemp);
    MockedFan* mockedFan = new MockedFan();
    std::shared_ptr<MockedFan> uMockedFan(mockedFan);
    BBQFanOnly* bbqFanOnly = new BBQFanOnly(std::move(uMockedTemp), std::move(uMockedFan));
    bbqFanOnly->init();
    bbqFanOnly->setPoint(130);
    mockedTemp->set(oven.temperature()); // set temperature sensor to oven temperature

    std::cout << "i,Temperature,setPoint,fan,lastError,change\n";

    for (int i = 0; i < 12000000; i = i + 100) {
        millisStubbed = i;
        oven.airFlow(mockedFan->mockedSpeed()*.8); // Set airflow of oven to fanspeed
        mockedTemp->set(oven.temperature()); // set temperature sensor to oven temperature
        oven.handle();

        if (i % 5000 == 0 && i > 0) {
            bbqFanOnly->handle();
            std::cout
                    << i / 1000 << ","
                    << oven.temperature() << ","
                    << bbqFanOnly->setPoint() << ","
                    << mockedFan->mockedSpeed() << ","
                    << bbqFanOnly->lastErrorInput() << ","
                    << bbqFanOnly->tempChangeInput() << ",";

            for (int i = 0; i < 20; i++) {
                std::cout << (30 + i) << ":" << bbqFanOnly->ruleFired(30 + i) << ",";

            };

            std::cout << "\n";
        }
    }
}



TEST_CASE("Single test", "[SINGLE][.]") {
    MockedTemperature* mockedTemp = new MockedTemperature(130.0);
    std::shared_ptr<TemperatureSensor> uMockedTemp(mockedTemp);
    MockedFan* mockedFan = new MockedFan();
    std::shared_ptr<MockedFan> uMockedFan(mockedFan);
    BBQFanOnly* bbqFanOnly = new BBQFanOnly(std::move(uMockedTemp), std::move(uMockedFan));
    bbqFanOnly->init();
    bbqFanOnly->setPoint(130);
    mockedTemp->set(130.0); // set temperature sensor to oven temperature

    std::cout << "i,Temperature,setPoint,fan,deltaError,lastError,change\n";


    for (int i = 0; i < 120; i = i + 100) {
        bbqFanOnly->handle();
        std::cout
                << mockedTemp->get() << ","
                << bbqFanOnly->setPoint() << ","
                << mockedFan->speed() << ","
                << bbqFanOnly->lastErrorInput() << ","
                << bbqFanOnly->tempChangeInput() << "\n";
    }
}

TEST_CASE("Should convert char values to other types", "[propertyvalue][ALL][.]") {
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

SCENARIO("shouldDetectLidOpen", "[controller]") {
    GIVEN("a BBQ at 130 degrees") {
        MockedTemperature* mockedTemp = new MockedTemperature(130);
        std::shared_ptr<TemperatureSensor> uMockedTemp(mockedTemp);
        MockedFan* mockedFan = new MockedFan();
        std::shared_ptr<MockedFan> uMockedFan(mockedFan);
        BBQFanOnly* bbqFanOnly = new BBQFanOnly(std::move(uMockedTemp), std::move(uMockedFan));
        bbqFanOnly->init();
        bbqFanOnly->setPoint(180.0f);

        bbqFanOnly->handle();
        bbqFanOnly->handle();
        REQUIRE(mockedTemp->get() == Approx(130.0));
        REQUIRE(bbqFanOnly->lidOpen() == false);

        WHEN("When temp lowers 1 degree") {
            mockedTemp->set(mockedTemp->get() - 1);
            bbqFanOnly->handle();
            THEN("lid should not have been detected as open") {
                REQUIRE(bbqFanOnly->lidOpen() == false);
                REQUIRE(mockedFan->speed() == Approx(77.72523));
            }
        }

        WHEN("When temp lowers 3 degree") {
            mockedTemp->set(mockedTemp->get() - 3);
            bbqFanOnly->handle();
            THEN("lid should have been detected as open") {
                REQUIRE(bbqFanOnly->lidOpen() == true);
                REQUIRE(mockedFan->speed() == 0);
            }
            WHEN("When temp stays the same") {
                bbqFanOnly->handle();
                THEN("lid should still have the detected as open") {
                    REQUIRE(bbqFanOnly->lidOpen() == true);
                    REQUIRE(mockedFan->speed() == 0);
                }
            }
            WHEN("When temp rises 0.5 degrees") {
                mockedTemp->set(mockedTemp->get() + 0.5);
                bbqFanOnly->handle();
                THEN("lid should still have the detected as open") {
                    REQUIRE(bbqFanOnly->lidOpen() == true);
                    REQUIRE(mockedFan->speed() == 0);
                }
                WHEN("When lid open fanspeed is set to 25") {
                    BBQFanOnlyConfig c;
                    c.fan_speed_lid_open = 25;
                    bbqFanOnly->config(c);
                    THEN("lid should still have the detected as open with new fan speed") {
                        bbqFanOnly->handle();
                        REQUIRE(bbqFanOnly->lidOpen() == true);
                        REQUIRE(mockedFan->speed() == Approx(25.0));
                    }
                }
                WHEN("When lid open fanspeed is set to -1") {
                    BBQFanOnlyConfig c;
                    c.fan_speed_lid_open = -1;
                    bbqFanOnly->config(c);
                    THEN("lid should still have been detected as open while keeping current fan speed") {
                        bbqFanOnly->handle();
                        bbqFanOnly->handle();
                        REQUIRE(bbqFanOnly->lidOpen() == true);
                        REQUIRE(mockedFan->speed() == Approx(77.66204f));
                    }
                }
            }
            WHEN("When temp rises 3 degrees") {
                mockedTemp->set(mockedTemp->get() + 3);
                bbqFanOnly->handle();
                THEN("lid should have reset to closed") {
                    REQUIRE(bbqFanOnly->lidOpen() == false);
                    REQUIRE(mockedFan->speed() == Approx(77.662));
                }
            }

        }

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

