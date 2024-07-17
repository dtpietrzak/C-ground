#include "utils.h"

char* get_schema_file_content(HttpResponse* http_response, char* schema_path,
                              char* db_path, QueryParams queries) {
  SString error_404;
  s_init(&error_404, "", 1024);
  s_compile(
      &error_404,
      "Schema not found for requested document: %s - use POST "
      "/schema?db=%s&key=%s with an example of your valid JSON schema and "
      "try again.",
      db_path, queries.db, queries.key);

  SString error_500;
  s_init(&error_500, "", 1024);
  s_compile(
      &error_500,
      "Failed to read schema data for the "
      "requested document: %s - use POST /schema?db=%s&key=%s with an example "
      "of your valid JSON schema and try again.",
      db_path, queries.db, queries.key);

  char* schema_file_content = get_file_content(
      http_response, schema_path, error_404.value, error_500.value);

  return schema_file_content;
}

JSON_Value* get_request_json_value(HttpResponse* http_response,
                                   char* string_to_parse) {
  JSON_Value* request_json_value = get_json_value(
      http_response, string_to_parse, "Failed to parse request JSON data");

  return request_json_value;
}