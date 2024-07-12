#include "delete.h"

char* handle_request_delete(HttpRequest* http_request) {
  QueryParams queries = validate_queries(http_request);
  // char* relative_path = derive_path("dbs", "test", "tost");

  return "404";
}