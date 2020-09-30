#pragma once

#include <stdint.h>
#include <vector>
#include <array>
#include <cstring>
#include <string>
#include <optparser.h>
#include <propertyUtils.h>
#include <iostream>
#include <algorithm>

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
        char buffer[32];
        strncpy(buffer, strKeyValues, sizeof(buffer));
        OptParser::get(buffer, ',', [&outValues, &totalValues](OptValue v) {
            if (v.pos() < SIZE) {
                outValues[v.pos()] = v;
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

inline float percentmap(float value, float out_max) {
    return value * out_max / 100.f;
}

template <typename T>
void getOptValue(const OptValue& value, const char* param,  T& n, const T& lower, const T& upper) {
    if (std::strcmp(value.key(), param) == 0) {
        int32_t n = between((int32_t)value, lower, upper);
    }
}
template <typename T>
bool getOptValue(const OptValue& value, const char* param, T& n) {
    bool found = false;

    if (std::strcmp(value.key(), param) == 0) {
        n = (T)value;
        found = true;
    }

    return found;
}

/**
 * parse a value from a string parameter
 * eg, it will get n1 from a string like : n=100, n1=200, n3=400
 */
template <typename T, std::size_t desiredCapacity>
bool getStringParameter(const char* value, const char* param, T& n) {
    static_assert(desiredCapacity > 0, "Must be > 0");
    char buffer[desiredCapacity];
    strncpy(buffer, value, desiredCapacity);
    bool found = false;
    OptParser::get(buffer, [&n, &param, &found](OptValue value) {
        found = found || getOptValue(value, param, n);
    });
    return found;
}

