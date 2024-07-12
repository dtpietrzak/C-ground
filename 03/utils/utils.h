#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

const char* INVALID_CHARS_DIRS_AND_FILES;

bool contains_invalid_chars(const char* str, const char* invalid_chars);

#endif  // UTILS_H