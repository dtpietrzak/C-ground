#include <sdb_utils.h>

char* get_schema_file_content(sdb_http_response_t* http_response, char* schema_path,
                              char* db_path, sdb_query_params_t queries);
JSON_Value* get_request_json_value(sdb_http_response_t* http_response,
                                   char* string_to_parse);