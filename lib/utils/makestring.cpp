#include "makestring.h"

#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <string>

/**
 * Create a new char* from input
 */
char* makeCString(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);
    return strdup(buffer);
}

std::string makeString(std::string format, ...) {
    va_list args, args_copy ;
    va_start(args, format) ;
    va_copy(args_copy, args) ;

    const auto sz = vsnprintf(nullptr, 0, format.c_str(), args) + 1 ;

    std::string result(sz, ' ') ;
    vsnprintf(&result.front(), sz, format.c_str(), args_copy) ;

    va_end(args_copy) ;
    va_end(args) ;
    return result ;
}
