#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG true
#define MEM_CHECK_SIZE 4096

void mem_init(const char* id);
void mem_free(const char* id);

#endif  // MEMORY_MEMORY_H