#include "upsert.h"

#define MAX_PATH_LENGTH 1024

void handle_request_insert(HttpRequest* http_request,
                           HttpResponse* http_response) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body.value, relative_path);
  switch (status) {
    case -1: {
      // Check if the file exists
      char* fileAccessIssue = check_file_access(relative_path, -2);
      if (fileAccessIssue != NULL) {
        http_response->status = 500;
        s_set(&http_response->body, fileAccessIssue);
        return;
      }
      http_response->status = 500;
      s_set(&http_response->body, "Failed to save data to document");
      break;
    }
    case 0:
      http_response->status = 201;
      s_set(&http_response->body, "Record inserted successfully");
      break;
    case 1:
      http_response->status = 204;
      s_set(&http_response->body, "Record updated successfully");
      break;
    default: {
      // Check if the file exists
      char* fileAccessIssue = check_file_access(relative_path, -2);
      if (fileAccessIssue != NULL) {
        http_response->status = 500;
        s_set(&http_response->body, fileAccessIssue);
        return;
      }
      http_response->status = 500;
      s_set(&http_response->body, "An unknown error occurred");
      break;
    }
  }
}