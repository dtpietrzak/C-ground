#include "query.h"

#define MAX_PATH_LENGTH 1024

char* handle_request_query(HttpRequest* http_request) {
  QueryParams queries = validate_queries(http_request);

  char relative_path[MAX_PATH_LENGTH];
#ifdef _WIN32
  snprintf(relative_path, sizeof(relative_path), "dbs\\%s\\%s", queries.db,
           queries.key);  // Windows uses backslashes
#else
  snprintf(relative_path, sizeof(relative_path), "./dbs/%s/%s", queries.db,
           queries.key);  // Unix-like systems use forward slashes
#endif

  // Read file content into string
  char* file_content = read_file_to_string(relative_path);
  if (file_content == NULL) {
    // Handle error
    return "404";
  }

  return file_content;
}