cmake_minimum_required(VERSION 3.0)

project(cmake_test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

ADD_DEFINITIONS(-DUNIT_TEST -DESP8266)

set(LIB_SOURCES
    ../.pio/libdeps/wemos/eFLL/FuzzyComposition.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzyIO.cpp
    ../.pio/libdeps/wemos/eFLL/Fuzzy.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzyInput.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzyOutput.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzyRule.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzyRuleAntecedent.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzyRuleConsequent.cpp
    ../.pio/libdeps/wemos/eFLL/FuzzySet.cpp
    ../.pio/libdeps/wemos/opt-parser/src/optparser.cpp
    ../lib/bbq/bbqfanonly.cpp
    ../lib/bbq/pwmventilator.cpp
    ../lib/bbq/ntcsensor.cpp
    ../lib/bbq/onoffventilator.cpp
    ../lib/bbq/analogin.cpp
    ../lib/bbq/numericknob.cpp
    ../lib/bbq/digitalknob.cpp
    ../lib/bbq/analogin.cpp
    ../lib/utils/propertyutils.cpp
    ../lib/utils/utils.cpp
    ../lib/utils/makestring.cpp
)

set(LIB_HEADERS
    stubs
    ../.pio/libdeps/wemos/eFLL
    ../.pio/libdeps/wemos/opt-parser/src
    ../lib/bbq
    ../lib/utils
    Arduino.h
)

include_directories(catch2 ${LIB_HEADERS})

# Make test executable
add_executable(tests main.cpp src/arduinostubs.hpp ${LIB_SOURCES})
target_link_libraries(tests Catch)
