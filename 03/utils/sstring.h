#ifndef SSTRING_H
#define SSTRING_H

#include <stdarg.h>
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
int s_init(SString* s_string, const char* value, uint16_t max_length);
int s_set(SString* s_string, const char* value);
int s_append(SString* s_string, const char* append_value);
int s_prepend(SString* s_string, const char* prepend_value);
int s_compile(SString* s_string, const char* format, ...);
int s_contains_chars(const SString* s_string, const char* chars_to_check);
int s_matches(SString* s_string, const char* string_to_check);
int s_before_and_after(SString* s_string, const char* before,
                       const char* after);
char* s_out(SString* s_string);

#endif  // SSTRING_H