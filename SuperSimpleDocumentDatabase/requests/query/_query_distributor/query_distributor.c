#include "query_distributor.h"

#define MAX_PATH_LENGTH 1024

typedef int (*QueryHandlerFunction)(sdb_http_request_t* http_request,
                                    sdb_http_response_t* http_response,
                                    sdb_query_params_t* queries);

// 0 = move on (thank you, next)
// 1 = exit early
int query(const char* query_type, sdb_query_params_t* queries,
          QueryHandlerFunction handler, const char* error_prefix,
          sdb_http_request_t* http_request,
          sdb_http_response_t* http_response) {
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

int handle_request_query(sdb_http_request_t* http_request,
                         sdb_http_response_t* http_response) {
  const char* params[] = {"db", "key", "value", "query", "limit", "sort"};
  // optionalParams[] = {"limit", "sort"};

  sdb_query_params_t queries =
      validate_and_parse_queries(http_request, params, 6);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  if (query("gtr", &queries, handle_query_number,
            "Comparator query failed: ", http_request, http_response)) {
    return 0;
  }

  if (query("lss", &queries, handle_query_number,
            "Comparator query failed: ", http_request, http_response)) {
    return 0;
  }

  if (query("gte", &queries, handle_query_number,
            "Comparator query failed: ", http_request, http_response)) {
    return 0;
  }

  if (query("lte", &queries, handle_query_number,
            "Comparator query failed: ", http_request, http_response)) {
    return 0;
  }
  // if (query("string", &queries, handle_request_query_les,
  //           "Less-than query failed: ", http_request, http_response)) {
  //   return 0;
  // }

  // etc...

  http_response->status = 400;
  s_set(&http_response->body, "Invalid query type");
  return 1;
}