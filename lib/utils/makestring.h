#pragma once

#include <string>

/**
 * Create a new char* from input with a maximum size of 512 bytes
 * Function is not thread safe as it uses a static buffer on the heap
 */
char* makeCString(const char* format, ...);

/**
 * Same as makeCString except for std::string
 */
std::string makeString(std::string format, ...);

