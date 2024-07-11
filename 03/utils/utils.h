#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool contains_invalid_chars(const char* str);
int make_directory(const char* path);
char* read_file_to_string(const char* relative_file_path);
int save_string_to_file(const char* data_string,
                         const char* relative_file_path);

#endif  // UTILS_H