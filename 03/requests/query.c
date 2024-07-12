#include "query.h"

#define MAX_PATH_LENGTH 1024

char* handle_request_query(HttpRequest* http_request) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  // Read file content into string
  char* file_content = read_file_to_string(relative_path);
  if (file_content == NULL) {
    // Handle error
    return "404";
  }

  return file_content;
}