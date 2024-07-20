#ifndef DIR_OPERATIONS_H
#define DIR_OPERATIONS_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void free_filenames(char** filenames, int count);
char** get_filenames(const char* dir_path, int* count);
int remove_directory(const char* dir_path);

#endif  // DIR_OPERATIONS_H