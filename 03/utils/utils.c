#include "utils.h"

// List of invalid characters for file names
const char* INVALID_CHARS_DIRS_AND_FILES = "\\/:*?\"<>|";

// Function to check if a string contains any invalid characters
bool contains_invalid_chars(const char* str, const char* invalid_chars) {
  for (const char* p = str; *p != '\0'; p++) {
    if (strchr(invalid_chars, *p) != NULL) {
      return true;
    }
  }
  return false;
}