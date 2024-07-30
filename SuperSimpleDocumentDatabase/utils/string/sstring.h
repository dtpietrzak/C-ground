#ifndef STRING_SSTRING_H
#define STRING_SSTRING_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char* value;
  uint16_t length;
  uint16_t max_length;
} SString;

void s_free(SString* s_string);
bool s_init(SString* s_string, const char* value, uint16_t max_length);
bool s_set(SString* s_string, const char* value);
bool s_append(SString* s_string, const char* append_value);
bool s_prepend(SString* s_string, const char* prepend_value);
bool s_compile(SString* s_string, const char* format, ...);
bool s_contains_chars(const SString* s_string, const char* chars_to_check);
bool s_matches(const SString* s_string, const char* string_to_check);
bool s_before_and_after(SString* s_string, const char* before,
                        const char* after);
char* s_out(SString* s_string);

#endif  // STRING_SSTRING_H