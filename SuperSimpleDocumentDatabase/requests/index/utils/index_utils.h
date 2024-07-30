#ifndef REQUESTS_INDEX_UTILS_H
#define REQUESTS_INDEX_UTILS_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sdb_utils.h>

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

typedef struct {
  JSON_Value* json_value;
  JSON_Array* array;
  size_t count;
} JSON_Array_With_Count;

int get_json_array_with_count(sdb_http_response_t* http_response,
                              char* string_to_parse,
                              JSON_Array_With_Count* json_array_with_count,
                              char* error_identity);

int insert_new_meta(sdb_http_response_t* http_response,
                    JSON_Array_With_Count request_array_with_count,
                    JSON_Array_With_Count existing_meta_array_with_count);

const char* get_document_dot_value_as_string(sdb_http_response_t* http_response,
                                             const char* file_path,
                                             const char* key);

void intToStr(int num, char* str);

#endif  // REQUESTS_INDEX_UTILS_H