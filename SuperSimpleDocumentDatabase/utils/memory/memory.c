#include "memory.h"

char* mem_check_string;

void mem_checker_init(void) {
  if (DEBUG) {
    mem_check_string = calloc(1, sizeof(char) * MEM_CHECK_SIZE);
  }
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

void mem_checker_alloc(const char* id, int debug_layer) {
  if (DEBUG && debug_layer >= DEBUG_LAYER) {
    ensure_mem_check_string_size(id);

    // generate a unique id that's 5 char long, and a return character
    char _id[10];
    _id[0] = ' ';
    _id[1] = '-';
    _id[2] = ' ';
    for (int i = 3; i < 8; i++) {
      _id[i] = (char)(rand() % 26 + 97);
    }
    _id[8] = '\n';
    _id[9] = '\0';

    printf("(%d) MEM - SET:  %s%s", debug_layer, id, _id);
    // append id to mem_check_string with a unique id and a newline
    strcat(mem_check_string, id);
    strcat(mem_check_string, _id);
  }
}

void mem_checker_free(const char* id, int debug_layer) {
  if (DEBUG && debug_layer >= DEBUG_LAYER) {
    ensure_mem_check_string_size(id);
    printf("(%d) MEM - FREE: %s\n", debug_layer, id);

    // remove the id from mem_check_string, including the 10 characters padding
    char* start = strstr(mem_check_string, id);
    if (start == NULL) {
      fprintf(stderr, "(%d) MEM - FREE: %s not found in mem_check_string",
              debug_layer, id);
      return;
    }
    char* end = strstr(start, "\n");
    if (end == NULL) {
      fprintf(stderr,
              "(%d) MEM - FREE: newline not found after %s in mem_check_string",
              debug_layer, id);
      return;
    }
    int length = end - start + 1;
    memmove(start, end + 1, strlen(end + 1) + 1);

    // zero out the remaining memory
    char* new_end = start + strlen(start);
    memset(new_end, 0, length);
  }
}

void mem_checker_check(int debug_layer) {
  if (DEBUG && debug_layer >= DEBUG_LAYER) {
    if (strlen(mem_check_string) > 0) {
      printf("(%d) MEM - CHECK: %s\n", debug_layer, mem_check_string);
    } else {
      printf("(%d) MEM - CHECK: No dynamic memory currently allocated\n\n",
             debug_layer);
    }
  }
}

void* mem_malloc(size_t __size, const char* id, int debug_layer) {
  mem_checker_alloc(id, debug_layer);
  return malloc(__size);
}

void* mem_calloc(size_t __count, size_t __size, const char* id,
                 int debug_layer) {
  mem_checker_alloc(id, debug_layer);
  return calloc(__count, __size);
}

void* mem_realloc(void* __ptr, size_t __size, const char* id, int debug_layer) {
  mem_checker_alloc(id, debug_layer);
  return realloc(__ptr, __size);
}

void mem_free(void* __ptr, const char* id, int debug_layer) {
  mem_checker_free(id, debug_layer);
  free(__ptr);
}