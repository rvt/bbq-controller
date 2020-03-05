#pragma once

#include <stdint.h>
#include <vector>
#include <array>
#include <cstring>
#include <string>
#include <optparser.h>
#include <iostream>
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

/**
 * Split a string into an array
 */
template <typename T, std::size_t SIZE>
inline std::array<T, SIZE> splitString(
    const std::string& toSplit,
    const char delim,
    std::function<T(const std::string&)> conversionFunction) {

    size_t start;
    size_t end = 0;
    std::array<T, SIZE> data;
    uint8_t num = 0;

    while (num < SIZE && (start = toSplit.find_first_not_of(delim, end)) != std::string::npos) {
        end = toSplit.find(delim, start);
        std::string value = toSplit.substr(start, end - start);
        T converted = conversionFunction(value);
        std::cout << converted << "\n";
        data[num] = converted;
        num++;
    }

    return data;
}


//

/**
 * Map input range to output range
 */
inline float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

/**
 * Ensure that a value is between and including a minimum and a maximum value
 */
template <typename T>
inline T between(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
}
