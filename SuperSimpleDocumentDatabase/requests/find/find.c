#include "find.h"

#define MAX_PATH_LENGTH 1024

int handle_request_find(sdb_http_request_t* http_request,
                        sdb_http_response_t* http_response) {
  const char* params[] = {"db", "key", "value"};

  sdb_query_params_t queries =
      validate_and_parse_queries(http_request, params, 3);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  char* index_path =
      derive_path(4, "index", queries.db, queries.key, queries.value);

  // if db_path is an error message
  if (index_path == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Failed to derive path");
    return 1;
  }

  // Check if the file exists
  char* file_access_issue = check_file_access(index_path, 1);
  if (file_access_issue != NULL) {
    if (strcmp(file_access_issue, "Document does not exist") == 0) {
      http_response->status = 404;
    } else {
      http_response->status = 500;
    }
    s_compile(&http_response->body, "%s: %s", file_access_issue, index_path);
    return 1;
  }

  // Read file content into string
  const char* file_content = read_file_to_string(index_path);
  if (file_content == NULL) {
    // 500 here even in the case of the file not existing
    // because we're checking that above
    http_response->status = 500;
    s_compile(&http_response->body,
              "Failed to read data from the requested document: %s",
              index_path);
    return 1;
  }

  http_response->status = 200;
  s_set(&http_response->body, file_content);
  return 0;

  // if (contains_periods(queries.id)) {
  //   // remove the before the first dot and the first dot itself
  //   char* json_id = strchr(queries.id, '.') + 1;

  //   // parse the json to get its value
  //   JSON_Value* json = json_parse_string_with_comments(file_content);
  //   JSON_Object* json_object = json_value_get_object(json);
  //   JSON_Value* value = json_object_dotget_value(json_object, json_id);
  //   if (value == NULL) {
  //     http_response->status = 404;
  //     s_compile(&http_response->body, "id not found in document: %s",
  //               queries.id);
  //     return 1;
  //   } else {
  //     http_response->status = 200;
  //     s_set(&http_response->body, json_serialize_to_string(value));
  //     return 0;
  //   }
  // } else {
  //   // get the whole object, but filter out any comments
  //   // from the JSON and minify it (done via parson)
  //   JSON_Value* json = json_parse_string_with_comments(file_content);
  //   if (json == NULL) {
  //     http_response->status = 500;
  //     s_compile(&http_response->body, "Failed to parse JSON");
  //     return 1;
  //   } else {
  //     http_response->status = 200;
  //     s_set(&http_response->body, json_serialize_to_string(json));
  //     return 0;
  //   }
  // }
}