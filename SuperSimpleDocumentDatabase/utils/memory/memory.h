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

void mem_check_init(void);
void mem_init(const char* id, int debug_layer);
void mem_free(const char* id, int debug_layer);
void mem_check(int debug_layer);

#endif  // MEMORY_MEMORY_H