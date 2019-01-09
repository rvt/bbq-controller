#include "utils.h"


std::vector<float> getConfigVector(
    const char* lookForKey,
    const char* strKeyValues,
    std::vector<float> inValues,
    bool* loaded) {

    if (strncmp(strKeyValues, lookForKey, strlen(lookForKey)) == 0) {
        const char* strValues = strstr(strKeyValues, "=");
        std::vector<float> outValues;
        OptParser::get(&strValues[1], ",", [&](OptValue v) {
            outValues.push_back(v.asFloat());
        });

        if (inValues.size() == outValues.size()) {
            if (loaded != nullptr) {
                *loaded = true;
            }

            return outValues;
        }
    }

    return inValues;
}
