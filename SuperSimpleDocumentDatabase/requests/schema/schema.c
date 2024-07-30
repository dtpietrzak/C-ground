#include "schema.h"

#define MAX_PATH_LENGTH 1024

int handle_request_schema(sdb_http_request_t* http_request,
                          sdb_http_response_t* http_response) {
  if (contains_invalid_chars(http_request->body.value,
                             INVALID_CHARS_REQUEST_BODY)) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid characters in request body");
    return 1;
  }

  const char* params[] = {"db"};
  sdb_query_params_t queries =
      validate_and_parse_queries(http_request, params, 1);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  // char* db_path = derive_path("db", queries.db, queries.id);
  char* schema_path = derive_path(2, "schema", queries.db);

  JSON_Value* json_value =
      json_parse_string_with_comments(http_request->body.value);
  if (json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid JSON");
    return 1;
  }

  char* updated_json = json_serialize_to_string_pretty(json_value);
  s_set(&http_request->body, updated_json);
  json_free_serialized_string(updated_json);
  json_value_free(json_value);

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body.value, schema_path);
  switch (status) {
    case -1: {
      // Check for file write access
      sdb_stater_t* stater_write_access = calloc(1, sizeof(sdb_stater_t));
      stater_write_access->error_body =
          "Schema document does not have write permissions";
      stater_write_access->error_status = 500;
      if (!fs_file_access_sync(http_response, schema_path, stater_write_access,
                               W_OK)) {
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body,
                "An unknown schema file write error has occured: %s",
                schema_path);
      return 1;
    }
    case 0:
      http_response->status = 201;
      s_compile(&http_response->body, "\"Schema inserted successfully: %s\"",
                schema_path);
      return 0;
    case 1:
      http_response->status = 204;
      s_compile(&http_response->body, "\"Schema updated successfully: %s\"",
                schema_path);
      return 0;
    default: {
      // Check for file write access
      sdb_stater_t* stater_write_access = calloc(1, sizeof(sdb_stater_t));
      stater_write_access->error_body =
          "Schema document does not have write permissions";
      stater_write_access->error_status = 500;
      if (!fs_file_access_sync(http_response, schema_path, stater_write_access,
                               W_OK)) {
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body,
                "An unknown schema file write error has occured: %s",
                schema_path);
      return 1;
    }
  }
}