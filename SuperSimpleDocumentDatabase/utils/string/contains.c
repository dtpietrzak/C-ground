#include "contains.h"

// TODO
// can probably get around these with char encoding like URL encoding
// convert the whole request body and query params via url encoding
// then convert all get requests to back to normal

// List of invalid characters for file names
const char* INVALID_CHARS_DIRS_AND_FILES = "\\/:*?\"<>|";
const char* INVALID_CHARS_REQUEST_BODY = "\\/*?<>|";
const char* VALID_CHARS_NUMBERS = "0123456789";

// Function to check if a string contains any invalid characters
bool contains_invalid_chars(char* str, const char* invalid_chars) {
  for (const char* p = str; *p != '\0'; p++) {
    if (strchr(invalid_chars, *p) != NULL) {
      return true;
    }
  }
  return false;
}

bool contains_only_valid_chars(char* str, const char* valid_chars) {
  for (const char* p = str; *p != '\0'; p++) {
    printf("char: %c\n", *p);
    if (strchr(valid_chars, *p) == NULL) {
      return false;
    }
  }
  return true;
}

bool contains_periods(const char* str) {
  for (const char* p = str; *p != '\0'; p++) {
    if (*p == '.') {
      return true;
    }
  }
  return false;
}