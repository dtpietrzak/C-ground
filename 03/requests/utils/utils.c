#include "utils.h"

#include <stdbool.h>
#include <stdlib.h>

// List of invalid characters for file names
const char* invalid_chars = "\\/:*?\"<>|";

// Function to check if a string contains any invalid characters
bool contains_invalid_chars(const char* str) {
  for (const char* p = str; *p != '\0'; p++) {
    if (strchr(invalid_chars, *p) != NULL) {
      return true;
    }
  }
  return false;
}