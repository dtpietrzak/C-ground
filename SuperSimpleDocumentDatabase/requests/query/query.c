#include "query.h"

#define MAX_PATH_LENGTH 1024

// 0 = move on (thank you, next)
// 1 = exit early
int query(const char* query_type, QueryParams* queries,
          int (*handler)(HttpRequest*, HttpResponse*, QueryParams*),
          const char* error_prefix, HttpRequest* http_request,
          HttpResponse* http_response) {
  if (strcmp(query_type, queries->query) == 0) {
    if (handler(http_request, http_response, queries) == 0) {
      return 1;
    } else {
      char* error_message = s_out(&http_response->body);
      s_compile(&http_response->body, "\"%s%s\"", error_prefix, error_message);
      free(error_message);
      return 1;
    }
  }
  return 0;
}

int handle_request_query(HttpRequest* http_request,
                         HttpResponse* http_response) {
  char* requiredParams[] = {"db", "key", "value", "query"};

  QueryParams queries = validate_queries(http_request, requiredParams, 4);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  if (query("gtr", &queries, handle_query_gtr,
            "Greater-than query failed: ", http_request, http_response)) {
    return 0;
  }
  // if (query("les", &queries, handle_request_query_les,
  //           "Less-than query failed: ", http_request, http_response)) {
  //   return 0;
  // }
  // if (query("gtre", &queries, handle_request_query_gtre,
  //           "Greater-than-or-equal-to query failed: ", http_request,
  //           http_response)) {
  //   return 0;
  // }
  // if (query("lese", &queries, handle_request_query_lese,
  //           "Less-than-or-equal-to query failed: ", http_request,
  //           http_response)) {
  //   return 0;
  // }

  http_response->status = 400;
  s_set(&http_response->body, "Invalid query type");
  return 1;
}