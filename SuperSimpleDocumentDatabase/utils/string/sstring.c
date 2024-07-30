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

bool s_init(SString *s_string, const char *value, uint16_t max_length) {
  // Check for NULL pointers
  if (s_string == NULL || value == NULL) {
    return false;
  }

  // Initialize SString fields to safe defaults
  s_string->value = NULL;
  s_string->length = 0;
  s_string->max_length = max_length;

  // Calculate the length of the value
  s_string->length = strnlen(value, max_length);

  // Check if value length exceeds max_length
  if (strlen(value) != s_string->length) {
    return false;
  }

  // Allocate memory for the string (plus one for null terminator)
  s_string->value = (char *)malloc(s_string->length + 1);
  if (s_string->value == NULL) {
    return false;  // Allocation failed
  }
  s_string->value[0] = '\0';

  // Copy the value and null terminate
  strncpy(s_string->value, value, s_string->length);
  s_string->value[s_string->length] = '\0';

  return true;
}

bool s_set(SString *s_string, const char *value) {
  if (s_string->value != NULL) {
    free(s_string->value);
  }

  // Calculate the length of the value
  s_string->length = strnlen(value, s_string->max_length);

  // Check if value length exceeds max_length
  if (strlen(value) != s_string->length) {
    return false;
  }

  // Allocate memory for the string (plus one for null terminator)
  s_string->value = (char *)malloc(s_string->length + 1);
  if (s_string->value == NULL) {
    return false;  // Allocation failed
  }
  s_string->value[0] = '\0';

  // Copy the value and null terminate
  strncpy(s_string->value, value, s_string->length);
  s_string->value[s_string->length] = '\0';

  return true;
}

bool s_append(SString *s_string, const char *append_value) {
  // Ensure sstring and append_value are not NULL
  if (s_string == NULL) {
    printf("s_append error: s_string is NULL\n");
    return false;
  } else if (s_string->value == NULL) {
    printf("s_append error: s_string->value is NULL\n");
    return false;
  } else if (append_value == NULL) {
    printf("s_append error: append_value is NULL\n");
    return false;
  }

  // Ensure sstring is not corrupt
  if (strlen(s_string->value) != s_string->length) {
    printf("s_append error: s_string is corrupt, invalid ->length\n");
    return false;
  }

  // Calculate lengths
  int append_length = strlen(append_value);
  int new_length = s_string->length + append_length;

  // Check if new length exceeds max_length
  if (new_length > s_string->max_length) {
    printf("s_append error: new_length exceeds max_length\n");
    return false;
  }

  // Malloc a new one
  char *new_value = (char *)malloc(new_length + 1);
  if (new_value == NULL) {
    printf("s_append error: malloc failed\n");
    return false;  // Allocation failed
  }
  new_value[0] = '\0';

  // Append string onto newly allocated memory
  strncpy(new_value, s_string->value, s_string->length + 1);
  strncat(new_value, append_value, append_length + 1);
  new_value[new_length] = '\0';

  // Update sstring struct
  s_string->length = new_length;
  if (s_string->value != NULL) {
    free(s_string->value);
  }
  s_string->value = new_value;
  return true;
}

bool s_prepend(SString *s_string, const char *prepend_value) {
  // Ensure sstring and append_value are not NULL
  if (s_string == NULL || s_string->value == NULL || prepend_value == NULL) {
    return false;
  }

  // Ensure sstring is not corrupt
  if (strlen(s_string->value) != s_string->length) {
    return false;
  }

  // Calculate lengths
  int prepend_length = strlen(prepend_value);
  int new_length = s_string->length + prepend_length;

  // Check if new length exceeds max_length
  if (new_length > s_string->max_length) {
    return false;
  }

  // Malloc a new one
  char *new_value = (char *)malloc(new_length + 1);
  if (new_value == NULL) {
    return false;  // Allocation failed
  }
  new_value[0] = '\0';

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
  return true;
}

bool s_compile(SString *s_string, const char *format, ...) {
  if (s_string->value != NULL) {
    free(s_string->value);
  }
  // Allocate a temporary buffer
  s_string->value = (char *)malloc(s_string->max_length + 1);
  if (s_string->value == NULL) {
    return false;  // Allocation failed
  }
  s_string->value[0] = '\0';

  va_list args;
  va_start(args, format);
  int result =
      vsnprintf(s_string->value, s_string->max_length + 1, format, args);
  va_end(args);
  if (result == 0) {
    fprintf(stderr, "s_compile error: 0 result %d\n", result);
    return false;
  }
  if (result > s_string->max_length) {
    fprintf(stderr, "s_compile error: result too large: %d, %d\n",
            s_string->max_length, result);
    return false;
  }
  if (result < 0) {
    fprintf(stderr, "s_compile error: negative result%d\n", result);
    return false;
  }

  s_string->length = strlen(s_string->value);
  return true;
}

bool s_contains_chars(const SString *s_string, const char *chars_to_check) {
  for (const char *p = s_string->value; *p != '\0'; p++) {
    if (strchr(chars_to_check, *p) != NULL) {
      return true;
    }
  }
  return false;
}

bool s_matches(const SString *s_string, const char *string_to_check) {
  if (strlen(string_to_check) > s_string->max_length) {
    return false;
  }
  if (strncmp(s_string->value, string_to_check, s_string->max_length) == 0) {
    return true;
  } else {
    return false;
  }
}

bool s_before_and_after(SString *s_string, const char *before,
                        const char *after) {
  // Ensure sstring and before/after are not NULL
  if (s_string == NULL || s_string->value == NULL || before == NULL ||
      after == NULL) {
    return false;
  }

  // Ensure sstring is not corrupt
  if (strlen(s_string->value) != s_string->length) {
    return false;
  }

  // Calculate new length
  int new_length = strlen(before) + strlen(after) + s_string->length;
  if (new_length > s_string->max_length) {
    return false;
  }

  // Allocate memory for the string (plus one for null terminator)
  char *new_value = (char *)malloc(new_length + 1);
  if (new_value == NULL) {
    return false;
  }
  new_value[0] = '\0';

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
  return true;
}

char *s_out(SString *s_string) {
  char *out = (char *)malloc(s_string->length + 1);
  if (out == NULL) {
    return NULL;
  }
  out[0] = '\0';
  strncpy(out, s_string->value, s_string->length);
  out[s_string->length] = '\0';
  return out;
}