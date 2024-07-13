#include "delete.h"

void handle_request_delete(HttpRequest* http_request,
                           HttpResponse* http_response) {
  QueryParams queries = validate_queries(http_request);
  char* relative_path = derive_path("dbs", queries.db, queries.key);

  http_response->status = 200;
  s_set(&http_response->body, relative_path);
}