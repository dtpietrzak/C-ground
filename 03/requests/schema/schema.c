#include "schema.h"

#define MAX_PATH_LENGTH 1024

int handle_request_schema(HttpRequest* http_request,
                          HttpResponse* http_response) {
  char* requiredParams[] = {"db"};
  QueryParams queries = validate_queries(http_request, requiredParams, 1);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  // char* db_path = derive_path("db", queries.db, queries.key);
  char* schema_path = derive_path_to_location("schema", queries.db);

  JSON_Value* json_value =
      json_parse_string_with_comments(http_request->body.value);
  if (json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid JSON");
    return 1;
  }

  char* updated_json = json_serialize_to_string_pretty(json_value);
  s_set(&http_request->body, updated_json);
  json_free_serialized_string(updated_json);
  json_value_free(json_value);

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body.value, schema_path);
  switch (status) {
    case -1: {
      // Check if the file exists
      char* file_access_issue = check_file_access(schema_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "%s: %s", file_access_issue,
                  schema_path);
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "%s", schema_path);
      return 1;
    }
    case 0:
      http_response->status = 201;
      s_compile(&http_response->body, "\"Schema inserted successfully: %s\"",
                schema_path);
      return 0;
    case 1:
      http_response->status = 204;
      s_compile(&http_response->body, "\"Schema updated successfully: %s\"",
                schema_path);
      return 0;
    default: {
      // Check if the file exists
      char* file_access_issue = check_file_access(schema_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "%s: %s", file_access_issue,
                  schema_path);
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "An unknown error occurred: %s",
                schema_path);
      return 1;
    }
  }
}