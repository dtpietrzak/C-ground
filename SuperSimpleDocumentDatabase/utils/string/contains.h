#ifndef STRING_CONTAINS_H
#define STRING_CONTAINS_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../global.c"

const char* INVALID_CHARS_DIRS_AND_FILES;
const char* INVALID_CHARS_REQUEST_BODY;
const char* VALID_CHARS_NUMBERS;

bool contains_invalid_chars(char* str, const char* invalid_chars);
bool contains_only_valid_chars(char* str, const char* valid_chars);
bool contains_periods(const char* str);

#endif  // STRING_CONTAINS_H