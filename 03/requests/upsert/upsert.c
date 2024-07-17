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
      get_json_value(http_response, http_request->body.value);
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

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body.value, db_path);
  switch (status) {
    case -1: {
      // Check if the file exists
      char* file_access_issue = check_file_access(db_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
                  db_path);
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "Failed to save data to document: %s",
                db_path);
      return 1;
    }
    case 0:
      http_response->status = 201;
      s_compile(&http_response->body, "\"Document inserted successfully: %s\"",
                db_path);
      return 0;
    case 1:
      http_response->status = 204;
      s_compile(&http_response->body, "\"Document updated successfully: %s\"",
                db_path);
      return 0;
    default: {
      // Check if the file exists
      char* file_access_issue = check_file_access(db_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
                  db_path);
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "An unknown error occurred: %s", db_path);
      return 1;
    }
  }
}
