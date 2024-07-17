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

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

int compile_doc_change(HttpResponse* http_response, HttpRequest* http_request,
                       char* db_path, QueryParams queries,
                       char* schema_file_content,
                       JSON_Value* request_json_value);