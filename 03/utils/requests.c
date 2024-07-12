#include "requests.h"

QueryParams invalidQueries = {
    .valid = false,
    .key = "",
    .db = "",
};

QueryParams validate_queries(HttpRequest* http_request) {
  QueryParams queryParams;
  for (int i = 0; i < http_request->num_queries; i++) {
    if (!strcmp(http_request->queries[i][0], "key")) {
      queryParams.key = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "db")) {
      queryParams.db = http_request->queries[i][1];
    }
  }
  if (queryParams.key == NULL) {
    return invalidQueries;
  } else if (contains_invalid_chars(queryParams.key,
                                    INVALID_CHARS_DIRS_AND_FILES)) {
    return invalidQueries;
  } else if (queryParams.db == NULL) {
    return invalidQueries;
  } else if (contains_invalid_chars(queryParams.db,
                                    INVALID_CHARS_DIRS_AND_FILES)) {
    return invalidQueries;
  }
  return queryParams;
}