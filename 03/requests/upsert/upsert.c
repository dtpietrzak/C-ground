#include "upsert.h"

#define MAX_PATH_LENGTH 1024

int handle_request_upsert(HttpRequest* http_request,
                          HttpResponse* http_response) {
  char* requiredParams[] = {"key", "db"};
  QueryParams queries = validate_queries(http_request, requiredParams, 2);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  char* db_path = derive_path_to_item("db", queries.db, queries.key);
  if (db_path == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Failed to derive db path");
    return 1;
  }
  char* schema_path = derive_path_to_location("schema", queries.db);
  if (schema_path == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Failed to derive schema path");
    return 1;
  }

  JSON_Value* request_json_value =
      get_request_json_value(http_response, http_request->body.value);
  if (request_json_value == NULL) return 1;

  char* schema_file_content =
      get_schema_file_content(http_response, schema_path, db_path, queries);
  if (schema_file_content == NULL) return 1;

  if (contains_periods(queries.key)) {
    // HANDLE DOT NOTATION UPSERT
    int compile_status = compile_dot_notation_change(
        http_response, http_request, db_path, queries, schema_file_content,
        request_json_value);
    if (compile_status == 1) return 1;
  } else {
    // HANDLE ENTIRE DOC UPSERT
    int compile_status =
        compile_doc_change(http_response, http_request, db_path, queries,
                           schema_file_content, request_json_value);
    if (compile_status == 1) return 1;
  }

  json_value_free(request_json_value);

  int save_status = save_string(http_response, http_request->body.value,
                                db_path, "\"Document inserted successfully\"",
                                "\"Document updated successfully\"",
                                "Failed to save data to document");
  if (save_status == -1) return 1;
  return 0;
}
