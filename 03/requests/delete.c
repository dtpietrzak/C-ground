#include "delete.h"

void handle_request_delete(HttpRequest* http_request,
                           HttpResponse* http_response) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  // Check if the file exists
  char* file_access_issue = check_file_access(relative_path, 1);
  if (file_access_issue != NULL) {
    if (strcmp(file_access_issue, "Document does not exist") == 0) {
      http_response->status = 404;
    } else {
      http_response->status = 500;
    }

    s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
              relative_path);
    return;
  }

  // Try to delete the file
  if (remove(relative_path) == 0) {
    http_response->status = 200;
    s_compile(&http_response->body, "\"Removed %s successfully\"",
              relative_path);
  } else {
    http_response->status = 500;
    s_compile(&http_response->body, "\"Failed to remove %s\"", relative_path);
  }
}