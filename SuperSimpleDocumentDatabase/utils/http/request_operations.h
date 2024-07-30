#ifndef HTTP_REQUEST_OPS_H
#define HTTP_REQUEST_OPS_H

#include <stdbool.h>
#include <string.h>

#include "../fs/file_operations.h"
#include "../libs/parson.h"
#include "../request_state/request_state.h"
#include "../string/contains.h"

char* get_file_content(sdb_http_response_t* http_response,
                       const char* relative_path, const char* error_message_404,
                       const char* error_message_500);

JSON_Value* get_json_value(sdb_http_response_t* http_response,
                           char* string_to_parse, char* error_message);

int save_string(sdb_http_response_t* http_response, const char* string_to_save,
                const char* relative_path, const char* success_message_201,
                const char* success_message_204, char* error_message_500);

#endif  // HTTP_REQUEST_OPS_H