#ifndef UTILS_REQUESTS_H
#define UTILS_REQUESTS_H

#include <stdbool.h>
#include <string.h>

#include "../http.h"
#include "file_operations.h"
#include "parson.h"
#include "utils.h"

typedef struct {
  char* invalid;
  char* key;
  char* db;
} QueryParams;

QueryParams validate_queries(HttpRequest* http_request, char* requiredParams[],
                             int num_required);

char* get_file_content(HttpResponse* http_response, char* relative_path,
                       const char* error_message_404,
                       const char* error_message_500);

JSON_Value* get_json_value(HttpResponse* http_response, char* string_to_parse,
                           char* error_message);

int save_string(HttpResponse* http_response, const char* string_to_save,
                char* relative_path, const char* success_message_201,
                const char* success_message_204, char* error_message_500);

#endif  // UTILS_REQUESTS_H