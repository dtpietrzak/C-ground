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

int compile_dot_notation_change(sdb_http_response_t* http_response,
                                sdb_http_request_t* http_request, char* db_path,
                                sdb_query_params_t queries, char* schema_file_content,
                                JSON_Value* request_json_value);