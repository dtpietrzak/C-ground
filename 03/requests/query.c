#include "query.h"

#define MAX_PATH_LENGTH 1024

void handle_request_query(HttpRequest* http_request,
                          HttpResponse* http_response) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  // Read file content into string
  char* file_content = read_file_to_string(relative_path);
  if (file_content == NULL) {
    http_response->status = 404;
    s_set(&http_response->body,
          "Failed to read data from the requested document");
  }

  http_response->status = 200;
  s_set(&http_response->body, file_content);
}