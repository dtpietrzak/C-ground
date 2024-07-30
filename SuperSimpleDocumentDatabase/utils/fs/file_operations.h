#ifndef UTILS_FILE_OPS_H
#define UTILS_FILE_OPS_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../global.c"
#include "../string/contains.h"

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#include <unistd.h>
#endif

char* derive_path(int num_args, ...);
int make_directory(const char* path);
char* read_file_to_string(const char* relative_file_path);
int save_string_to_file(const char* data_string,
                        const char* relative_file_path);
char* check_file_access(const char* path, int8_t level);

#endif  // UTILS_FILE_OPS_H