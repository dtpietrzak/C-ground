#include "upsert.h"

#define MAX_PATH_LENGTH 1024

char* handle_request_insert(HttpRequest* http_request) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body, relative_path);
  switch (status) {
    case -1:
      return "500";
      break;
    case 0:
      return "201";
      break;
    case 1:
      return "204";
      break;
    default:
      return "500";
      break;
  }
}