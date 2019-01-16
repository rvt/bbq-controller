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

ADD_DEFINITIONS(-DUNIT_TEST)

set(LIB_SOURCES
    ../.piolibdeps/eFLL/FuzzyComposition.cpp
    ../.piolibdeps/eFLL/FuzzyIO.cpp
    ../.piolibdeps/eFLL/Fuzzy.cpp
    ../.piolibdeps/eFLL/FuzzyInput.cpp
    ../.piolibdeps/eFLL/FuzzyOutput.cpp
    ../.piolibdeps/eFLL/FuzzyRule.cpp
    ../.piolibdeps/eFLL/FuzzyRuleAntecedent.cpp
    ../.piolibdeps/eFLL/FuzzyRuleConsequent.cpp
    ../.piolibdeps/eFLL/FuzzySet.cpp
    ../.piolibdeps/opt-parser/src/optparser.cpp
    ../lib/bbq/bbqfanonly.cpp
    ../lib/bbq/analogin.cpp
    ../lib/utils/utils.cpp
    ../lib/utils/makestring.cpp
)

set(LIB_HEADERS
    ../.piolibdeps/eFLL
    ../.piolibdeps/opt-parser/src
    ../lib/bbq
    ../lib/utils
)

include_directories(catch2 ${LIB_HEADERS})

# Make test executable
add_executable(tests main.cpp src/arduinostubs.hpp ${LIB_SOURCES})
target_link_libraries(tests Catch)
