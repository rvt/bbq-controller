#pragma once

#include <stdint.h>
#include <vector>
#include <array>
#include <cstring>
#include <optparser.h>
#include <algorithm>


std::vector<float> getConfigVector(
    const char* lookForKey,
    const char* strKeyValues,
    std::vector<float> inValues,
    bool* loaded = nullptr);

// TODO make thin template
template<std::size_t SIZE>
std::array<float, SIZE> getConfigArray(
    const char* lookForKey,
    const char* givenKey,
    const char* strKeyValues,
    std::array<float, SIZE> inValues,
    bool* loaded = nullptr) {

    if (strcmp(lookForKey, givenKey) == 0) {
        int totalValues = 0;
        std::array<float, SIZE> outValues;
        OptParser::get(strKeyValues, ",", [&outValues, &totalValues](OptValue v) {
            if (v.pos() < SIZE) {
                outValues[v.pos()] = v.asFloat();
                totalValues++;
            }
        });

        if (totalValues == SIZE) {
            return outValues;
        }
    }

    return inValues;
}

template <typename T>
static T between(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
}
