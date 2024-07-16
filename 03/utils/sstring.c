#include "sstring.h"

void s_free(SString *sstring) {
  if (sstring != NULL) {
    if (sstring->value != NULL) {
      free(sstring->value);
      sstring->value = NULL;
    }
    sstring->length = 0;
    sstring->max_length = 0;
  }
}

int s_init(SString *s_string, const char *value, uint16_t max_length) {
  // Check for NULL pointers
  if (s_string == NULL || value == NULL) {
    return 1;
  }

  // Initialize SString fields to safe defaults
  s_string->value = NULL;
  s_string->length = 0;
  s_string->max_length = max_length;

  // Calculate the length of the value
  s_string->length = strnlen(value, max_length);

  // Check if value length exceeds max_length
  if (strlen(value) != s_string->length) {
    return 1;
  }

  // Allocate memory for the string (plus one for null terminator)
  s_string->value = (char *)malloc(s_string->length + 1);
  if (s_string->value == NULL) {
    return 1;  // Allocation failed
  }

  // Copy the value and null terminate
  strncpy(s_string->value, value, s_string->length);
  s_string->value[s_string->length] = '\0';

  return 0;
}

int s_set(SString *s_string, const char *value) {
  if (s_string->value != NULL) {
    free(s_string->value);
  }

  // Calculate the length of the value
  s_string->length = strnlen(value, s_string->max_length);

  // Check if value length exceeds max_length
  if (strlen(value) != s_string->length) {
    return 1;
  }

  // Allocate memory for the string (plus one for null terminator)
  s_string->value = (char *)malloc(s_string->length + 1);
  if (s_string->value == NULL) {
    return 1;  // Allocation failed
  }

  // Copy the value and null terminate
  strncpy(s_string->value, value, s_string->length);
  s_string->value[s_string->length] = '\0';

  return 0;
}

int s_append(SString *s_string, const char *append_value) {
  // Ensure sstring and append_value are not NULL
  if (s_string == NULL || s_string->value == NULL || append_value == NULL) {
    return 1;
  }

  // Ensure sstring is not corrupt
  if (strlen(s_string->value) != s_string->length) {
    return 1;
  }

  // Calculate lengths
  int append_length = strlen(append_value);
  int new_length = s_string->length + append_length;

  // Check if new length exceeds max_length
  if (new_length > s_string->max_length) {
    return 1;
  }

  // Realloc to accommodate new length
  s_string->value = realloc(s_string->value, new_length + 1);
  if (s_string->value == NULL) {
    return 1;  // Allocation failed
  }

  // Append string onto newly allocated memory
  strncat(s_string->value, append_value, append_length);
  s_string->value[new_length] = '\0';

  // update sstring struct
  s_string->length = new_length;
  return 0;
}

int s_prepend(SString *s_string, const char *prepend_value) {
  // Ensure sstring and append_value are not NULL
  if (s_string == NULL || s_string->value == NULL || prepend_value == NULL) {
    return 1;
  }

  // Ensure sstring is not corrupt
  if (strlen(s_string->value) != s_string->length) {
    return 1;
  }

  // Calculate lengths
  int prepend_length = strlen(prepend_value);
  int new_length = s_string->length + prepend_length;

  // Check if new length exceeds max_length
  if (new_length > s_string->max_length) {
    return 1;
  }

  // Malloc a new one
  char *new_value = (char *)malloc(new_length + 1);
  if (new_value == NULL) {
    return 1;  // Allocation failed
  }

  // Append string onto newly allocated memory
  strncpy(new_value, prepend_value, prepend_length);
  strncat(new_value, s_string->value, s_string->length);
  new_value[new_length] = '\0';

  // Update sstring struct
  s_string->length = new_length;
  if (s_string->value != NULL) {
    free(s_string->value);
  }
  s_string->value = new_value;
  return 0;
}

int s_compile(SString *s_string, const char *format, ...) {
  if (s_string->value != NULL) {
    free(s_string->value);
  }
  // Allocate a temporary buffer
  s_string->value = (char *)malloc(s_string->max_length + 1);
  if (s_string->value == NULL) {
    return 1;  // Allocation failed
  }

  va_list args;
  va_start(args, format);
  int result =
      vsnprintf(s_string->value, s_string->max_length + 1, format, args);
  va_end(args);
  if (result == 0) {
    fprintf(stderr, "s_compile error: 0 result %d\n", result);
    return 1;
  }
  if (result > s_string->max_length) {
    fprintf(stderr, "s_compile error: result too large: %d, %d\n",
            s_string->max_length, result);
    return 1;
  }
  if (result < 0) {
    fprintf(stderr, "s_compile error: negative result%d\n", result);
    return 1;
  }

  s_string->length = strlen(s_string->value);
  return 0;
}

int s_contains_chars(SString *s_string, const char *chars_to_check) {
  for (const char *p = s_string->value; *p != '\0'; p++) {
    if (strchr(chars_to_check, *p) != NULL) {
      return 0;
    }
  }
  return 1;
}

int s_matches(SString *s_string, const char *string_to_check) {
  if (strlen(string_to_check) > s_string->max_length) {
    return 1;
  }
  if (strncmp(s_string->value, string_to_check, s_string->max_length) == 0) {
    return 0;
  } else {
    return 1;
  }
}

int s_before_and_after(SString *s_string, const char *before, const char *after) {
  // Ensure sstring and before/after are not NULL
  if (s_string == NULL || s_string->value == NULL || before == NULL ||
      after == NULL) {
    return 1;
  }

  // Ensure sstring is not corrupt
  if (strlen(s_string->value) != s_string->length) {
    return 1;
  }

  // Calculate new length
  int new_length = strlen(before) + strlen(after) + s_string->length;
  if (new_length > s_string->max_length) {
    return 1;
  }

  // Allocate memory for the string (plus one for null terminator)
  char *new_value = (char *)malloc(new_length + 1);
  if (new_value == NULL) {
    return 1;
  }

  // build the new string into malloc'd memory
  strcpy(new_value, before);
  strcat(new_value, s_string->value);
  strcat(new_value, after);

  // update sstring struct
  s_string->length = strlen(new_value);
  if (s_string->value != NULL) {
    free(s_string->value);
  }
  s_string->value = new_value;
  return 0;
}