#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG true
#define DEBUG_LAYER 1
// debug layers
// 1 - main / settings
// 2 - server
// 3 - request L2
// 4 - database L1
// 5 - database L2
// 6 - database L3
// 7 - deep

#define MEM_CHECK_SIZE 4096

void mem_checker_init(void);
void mem_checker_alloc(const char* id, int debug_layer);
void mem_checker_free(const char* id, int debug_layer);

void mem_checker_check(int debug_layer);

void* mem_malloc(size_t __size, const char* id, int debug_layer);
void* mem_calloc(size_t __count, size_t __size, const char* id,
                 int debug_layer);
void* mem_realloc(void* __ptr, size_t __size, const char* id, int debug_layer);

void mem_free(void* __ptr, const char* id, int debug_layer);

#endif  // MEMORY_MEMORY_H