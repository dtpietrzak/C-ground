#include "query.h"

#define MAX_PATH_LENGTH 1024

void handle_request_query(HttpRequest* http_request,
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

  // Read file content into string
  char* file_content = read_file_to_string(relative_path);
  if (file_content == NULL) {
    // 500 here even in the case of the file not existing
    // because we're checking that above
    http_response->status = 500;
    s_compile(&http_response->body,
              "\"Failed to read data from the requested document: %s\"",
              relative_path);
    return;
  }

  http_response->status = 200;
  s_set(&http_response->body, file_content);
}