#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../http.h"
#include "../../utils/file_operations.h"
#include "../../utils/parson.h"
#include "../../utils/requests.h"
#include "../../utils/utils.h"
#include "compile_dot_notation.h"
#include "compile_entire_doc.h"

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

char* get_schema_file_content(HttpResponse* http_response, char* schema_path,
                              char* db_path, QueryParams queries);
JSON_Value* get_json_value(HttpResponse* http_response, char* string_to_parse);