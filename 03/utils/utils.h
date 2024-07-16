#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../global.c"

const char* INVALID_CHARS_DIRS_AND_FILES;

bool contains_invalid_chars(const char* str, const char* invalid_chars);
bool contains_periods(const char* str);

#endif  // UTILS_H