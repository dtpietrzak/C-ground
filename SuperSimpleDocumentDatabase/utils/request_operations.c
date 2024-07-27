#include "request_operations.h"

// Validate the queries in the HTTP request
// If the queries are invalid, set the invalid message
// If the queries are valid, set the id and / or db name
//
// Returns the QueryParams struct
// if the queries are invalid, the invalid field will be set (not NULL)
QueryParams validate_queries(HttpRequest* http_request, char* required_params[],
                             int num_required) {
  // this is getting too large and should be refactored

  QueryParams queryParams = {
      .invalid = NULL,
      .id = NULL,
      .key = NULL,
      .value = NULL,
      .query = NULL,
      .db = NULL,
  };

  if (num_required != http_request->num_queries) {
    queryParams.invalid = "Invalid number of queries";
    return queryParams;
  }

  bool id_is_required = false;
  bool key_is_required = false;
  bool value_is_required = false;
  bool query_is_required = false;
  bool db_is_required = false;

  for (int i = 0; i < http_request->num_queries; i++) {
    if (strcmp(required_params[i], "id") == 0) {
      id_is_required = true;
    }
    if (strcmp(required_params[i], "key") == 0) {
      key_is_required = true;
    }
    if (strcmp(required_params[i], "value") == 0) {
      value_is_required = true;
    }
    if (strcmp(required_params[i], "query") == 0) {
      query_is_required = true;
    }
    if (strcmp(required_params[i], "db") == 0) {
      db_is_required = true;
    }

    if (!strcmp(http_request->queries[i][0], "id")) {
      queryParams.id = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "key")) {
      queryParams.key = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "value")) {
      queryParams.value = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "query")) {
      queryParams.query = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "db")) {
      queryParams.db = http_request->queries[i][1];
    }
  }

  if (id_is_required) {
    if (queryParams.id == NULL) {
      queryParams.invalid = "Id is missing";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.id,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid = "Id contains invalid characters";
      return queryParams;
    }
  }

  if (key_is_required) {
    if (queryParams.key == NULL) {
      queryParams.invalid = "Key is missing";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.key,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid = "Key contains invalid characters";
      return queryParams;
    }
  }

  if (value_is_required) {
    if (queryParams.value == NULL) {
      queryParams.invalid = "Value is missing";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.value,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid = "Value contains invalid characters";
      return queryParams;
    }
  }

  if (query_is_required) {
    if (queryParams.query == NULL) {
      queryParams.invalid = "Query is missing";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.query,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid = "Query contains invalid characters";
      return queryParams;
    } else if (
        // strcmp(queryParams.query, "new") != 0 &&
        // strcmp(queryParams.query, "old") != 0 &&
        strcmp(queryParams.query, "gtr") != 0  // &&
        // strcmp(queryParams.query, "les") != 0 &&
        // strcmp(queryParams.query, "gtre") != 0 &&
        // strcmp(queryParams.query, "lese") != 0
    ) {
      queryParams.invalid = "Query is invalid";
      return queryParams;
    }
  }

  if (db_is_required) {
    if (queryParams.db == NULL) {
      queryParams.invalid = "Db name is missing";
      return queryParams;
    } else if (contains_invalid_chars(queryParams.db,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      queryParams.invalid =
          "Db name contains invalid characters (cannot contain: "
          "\\/:*?\"<>|)";
      return queryParams;
    }
    if (contains_periods(queryParams.db)) {
      queryParams.invalid =
          "Db name contains invalid characters (cannot contain a period)";
      return queryParams;
    }
  }

  return queryParams;
}

// Get the content of a file and return it as a string
// If the file does not exist, set the response status to 404
// and set the response body to the error message
// If there is an issue reading the file, set the response status to 500
// and set the response body to the error message
//
// Returns the file content if successful, NULL if not
char* get_file_content(HttpResponse* http_response, const char* relative_path,
                       const char* error_message_404,
                       const char* error_message_500) {
  // Check if the file exists
  const char* schema_file_access_issue = check_file_access(relative_path, 1);
  if (schema_file_access_issue != NULL) {
    if (strcmp(schema_file_access_issue, "Document does not exist") == 0) {
      http_response->status = 404;
      s_set(&http_response->body, error_message_404);
    } else {
      http_response->status = 500;
      s_compile(&http_response->body, error_message_500);
    }
    return NULL;
  }

  char* schema_file = read_file_to_string(relative_path);
  if (schema_file == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, error_message_500);
    return NULL;
  }

  return schema_file;
}

// Parse the JSON string and return the JSON_Value
// If the JSON string is invalid, set the response status to 400
// and set the response body to the error message
//
// Returns the JSON_Value if successful, NULL if not
JSON_Value* get_json_value(HttpResponse* http_response, char* string_to_parse,
                           char* error_message) {
  JSON_Value* request_json_value =
      json_parse_string_with_comments(string_to_parse);
  if (request_json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, error_message);
  }

  return request_json_value;
}

// Save a string to a file
// If the file does not exist, set the response status to 500
// and set the response body to the error message
// If the file is new and saved successfully, set the response status to 201
// and set the response body to the success message
// If the file is updated and saved successfully, set the response status to 204
// and set the response body to the success message
//
// Returns 0 if the file is new and saved successfully
// Returns 1 if the file is updated and saved successfully
// Returns -1 if there is an issue saving the file
int save_string(HttpResponse* http_response, const char* string_to_save,
                const char* relative_path, const char* success_message_201,
                const char* success_message_204, char* error_message_500) {
  int status = save_string_to_file(string_to_save, relative_path);
  switch (status) {
    case -1: {
      // Check if the file exists
      const char* file_access_issue = check_file_access(relative_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        strcat(error_message_500, " - ");
        strcat(error_message_500, file_access_issue);
        s_set(&http_response->body, error_message_500);
        return -1;
      }
      http_response->status = 500;
      s_set(&http_response->body, error_message_500);
      return -1;
    }
    case 0:
      http_response->status = 201;
      s_set(&http_response->body, success_message_201);
      return 0;
    case 1:
      http_response->status = 204;
      s_set(&http_response->body, success_message_204);
      return 1;
    default: {
      // Check if the file exists
      char* file_access_issue = check_file_access(relative_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        strcat(error_message_500, " - ");
        strcat(error_message_500, file_access_issue);
        s_set(&http_response->body, error_message_500);
        return -1;
      }
      http_response->status = 500;
      s_set(&http_response->body, "An unknown error occurred");
      return -1;
    }
  }
}