#include "memory.h"

char* mem_check_string;

void mem_check_init() {
  mem_check_string = malloc(sizeof(char) * MEM_CHECK_SIZE);
}

void ensure_mem_check_string_size(const char* id) {
  // 5 for unique id, 3 for " - ", 1 for newline, 1 for null terminator
  int expected_padding_length = 10;
  int length = strlen(mem_check_string) + strlen(id) + expected_padding_length;
  if (length > MEM_CHECK_SIZE) {
    fprintf(stderr, "mem_check_string ran out of memory!");
    exit(1);
  }
}

void mem_init(const char* id) {
  if (DEBUG) {
    ensure_mem_check_string_size(id);
    printf("MEM - SET: %s", id);

    // generate a unique id that's 5 char long
    char _id[6];
    for (int i = 0; i < 5; i++) {
      _id[i] = (char)(rand() % 26 + 97);
    }
    _id[5] = '\0';

    // append id to mem_check_string with a unique id and a newline
    strcat(mem_check_string, id);
    strcat(mem_check_string, " - ");
    strcat(mem_check_string, _id);
    strcat(mem_check_string, "\n");
  }
}
void mem_free(const char* id) {
  if (DEBUG) {
    ensure_mem_check_string_size(id);
    printf("MEM - FREE: %s", id);

    // remove the id from mem_check_string, including the 10 characters padding
    char* start = strstr(mem_check_string, id);
    if (start == NULL) {
      fprintf(stderr, "MEM - FREE: %s not found in mem_check_string", id);
      return;
    }
    char* end = strstr(start, "\n");
    if (end == NULL) {
      fprintf(stderr,
              "MEM - FREE: newline not found after %s in mem_check_string", id);
      return;
    }
    int length = end - start + 1;
    memmove(start, end + 1, strlen(end + 1) + 1);

    // fill the gap with null terminators
    for (int i = 0; i < length; i++) {
      mem_check_string[strlen(mem_check_string) - i] = '\0';
    }
  }
}

void mem_check() { printf("MEM - CHECK: %s", mem_check_string); }