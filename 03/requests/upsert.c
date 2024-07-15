#include "upsert.h"

#define MAX_PATH_LENGTH 1024

void handle_request_insert(HttpRequest* http_request,
                           HttpResponse* http_response) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  JSON_Value* is_valid_json =
      json_parse_string_with_comments(http_request->body.value);
  if (is_valid_json == NULL) {
    http_response->status = 400;
    s_compile(&http_response->body, "\"Failed to save: Invalid JSON\"");
    return;
  }

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body.value, relative_path);
  switch (status) {
    case -1: {
      // Check if the file exists
      char* file_access_issue = check_file_access(relative_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
                  relative_path);
        return;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "\"Failed to save data to document: %s\"",
                relative_path);
      break;
    }
    case 0:
      http_response->status = 201;
      s_compile(&http_response->body, "\"Document inserted successfully: %s\"",
                relative_path);
      break;
    case 1:
      http_response->status = 204;
      s_compile(&http_response->body, "\"Document updated successfully: %s\"",
                relative_path);
      break;
    default: {
      // Check if the file exists
      char* file_access_issue = check_file_access(relative_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
                  relative_path);
        return;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "\"An unknown error occurred: %s\"",
                relative_path);
      break;
    }
  }
}