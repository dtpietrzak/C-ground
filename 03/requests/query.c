#include "query.h"

#define MAX_PATH_LENGTH 1024

void handle_request_query(HttpRequest* http_request,
                          HttpResponse* http_response) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  // Check if the file exists
  char* fileAccessIssue = check_file_access(relative_path, 1);
  if (fileAccessIssue != NULL) {
    http_response->status = 500;
    s_set(&http_response->body, fileAccessIssue);
    return;
  }

  // Read file content into string
  char* file_content = read_file_to_string(relative_path);
  if (file_content == NULL) {
    http_response->status = 404;
    s_set(&http_response->body,
          "Failed to read data from the requested document");
    return;
  }

  http_response->status = 200;
  s_set(&http_response->body, file_content);
}