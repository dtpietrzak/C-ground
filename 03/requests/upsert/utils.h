#include "../../utils/request_operations.h"

char* get_schema_file_content(HttpResponse* http_response, char* schema_path,
                              char* db_path, QueryParams queries);
JSON_Value* get_request_json_value(HttpResponse* http_response,
                                   char* string_to_parse);