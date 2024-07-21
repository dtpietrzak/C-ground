#include "utils.h"

// TODO
// can probably get around these with char encoding like URL encoding
// convert the whole request body and query params via url encoding
// then convert all get requests to back to normal

// List of invalid characters for file names
const char* INVALID_CHARS_DIRS_AND_FILES = "\\/:*?\"<>|";
const char* INVALID_CHARS_REQUEST_BODY = "\\/*?<>|";

// Function to check if a string contains any invalid characters
bool contains_invalid_chars(char* str, const char* invalid_chars) {
  for (const char* p = str; *p != '\0'; p++) {
    if (strchr(invalid_chars, *p) != NULL) {
      return true;
    }
  }
  return false;
}

bool contains_periods(const char* str) {
  for (const char* p = str; *p != '\0'; p++) {
    if (*p == '.') {
      return true;
    }
  }
  return false;
}