#include "requests.h"

QueryParams validate_queries(HttpRequest* http_request, char* required_params[],
                             int num_required) {
  QueryParams queryParams = {
      .invalid = NULL,
      .key = NULL,
      .db = NULL,
  };

  if (num_required != http_request->num_queries) {
    queryParams.invalid = "\"Invalid number of queries\"";
    return queryParams;
  }

  bool key_is_required = false;
  bool db_is_required = false;

  for (int i = 0; i < http_request->num_queries; i++) {
    if (strcmp(required_params[i], "key") == 0) {
      key_is_required = true;
    }
    if (strcmp(required_params[i], "db") == 0) {
      db_is_required = true;
    }

    if (!strcmp(http_request->queries[i][0], "key")) {
      queryParams.key = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "db")) {
      queryParams.db = http_request->queries[i][1];
    }
  }

  if (key_is_required) {
    if (queryParams.key == NULL) {
      queryParams.invalid = "\"Key is missing\"";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.key,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid = "\"Key contains invalid characters\"";
      return queryParams;
    }
  }

  if (db_is_required) {
    if (queryParams.db == NULL) {
      queryParams.invalid = "\"Db name is missing\"";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.db,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid =
          "\"Db name contains invalid characters (cannot contain: "
          "\\/:*?\"<>|)\"";
      return queryParams;
    }
    if (contains_periods(queryParams.db)) {
      queryParams.invalid =
          "\"Db name contains invalid characters (cannot contain a period)\"";
      return queryParams;
    }
  }

  return queryParams;
}