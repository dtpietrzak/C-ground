#include "utils.h"

char* get_schema_file_content(HttpResponse* http_response, char* schema_path,
                              char* db_path, QueryParams queries) {
  // Check if the file exists
  char* schema_file_access_issue = check_file_access(schema_path, 1);
  if (schema_file_access_issue != NULL) {
    if (strcmp(schema_file_access_issue, "Document does not exist") == 0) {
      http_response->status = 404;
      s_compile(&http_response->body,
                "Schema not found for requested document: %s - use POST "
                "/schema?db=%s&key=%s with an example of your valid JSON "
                "schema and try again.",
                db_path, queries.db, queries.key);
    } else {
      http_response->status = 500;
      s_compile(
          &http_response->body,
          "Failed to read schema data for the "
          "requested document: %s - use POST /schema?db=%s&key=%s with an "
          "example of your valid JSON schema and try again.",
          db_path, queries.db, queries.key);
    }

    return NULL;
  }

  char* schema_file = read_file_to_string(schema_path);
  if (schema_file == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Failed to read schema data for the "
              "requested document: %s - use POST /schema?db=%s&key=%s with an "
              "example of your valid JSON schema and try again.",
              db_path, queries.db, queries.key);
    return NULL;
  }

  return schema_file;
}

JSON_Value* get_json_value(HttpResponse* http_response, char* string_to_parse) {
  JSON_Value* request_json_value =
      json_parse_string_with_comments(string_to_parse);
  if (request_json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid JSON");
  }

  return request_json_value;
}