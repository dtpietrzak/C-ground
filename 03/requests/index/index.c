#include "index.h"

#define MAX_PATH_LENGTH 1024

int handle_request_index(HttpRequest* http_request,
                         HttpResponse* http_response) {
  char* requiredParams[] = {"db", "key"};
  QueryParams queries = validate_queries(http_request, requiredParams, 2);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  // char* db_path = derive_path("db", queries.db, queries.key);
  // const char* index_path = derive_path(3, "index", queries.db, queries.key);
  char* index_meta_path = derive_path(3, "index", queries.db, "_meta");

  const JSON_Value* request_body_json_value =
      get_json_value(http_response, http_request->body.value,
                     "Failed to parse request JSON data");
  if (request_body_json_value == NULL) return 1;

  const JSON_Array* request_body_json_array =
      json_value_get_array(request_body_json_value);
  if (request_body_json_array == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Request body must be an array");
    return 1;
  }

  size_t request_body_array_count =
      json_array_get_count(request_body_json_array);
  if (request_body_array_count == 0) {
    http_response->status = 400;
    s_set(&http_response->body, "Request body array must not be empty");
    return 1;
  }

  const JSON_Value* updated_json_value = json_value_init_array();
  if (updated_json_value == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to initialize updated JSON data");
    return 1;
  }

  char* existing_meta = get_file_content(http_response, index_meta_path,
                                         "Index meta file not found",
                                         "Failed to read index meta file");
  if (existing_meta == NULL) {
    if (http_response->status != 404) {
      // free the existing meta data
      return 1;
    }
    // Index meta file doesn't exist yet, no need to merge
    // just continue to saving the new index meta data
    updated_json_value = json_value_deep_copy(request_body_json_value);
  } else {
    // Index already exists, lets gooooooo!
    const JSON_Value* existing_meta_json_value =
        get_json_value(http_response, existing_meta,
                       "Failed to parse existing meta JSON data");
    if (existing_meta_json_value == NULL) return 1;

    JSON_Array* existing_meta_json_array =
        json_value_get_array(existing_meta_json_value);
    if (existing_meta_json_array == NULL) {
      http_response->status = 500;
      s_set(&http_response->body, "Existing meta data must be an array");
      return 1;
    }

    size_t existing_meta_array_count =
        json_array_get_count(existing_meta_json_array);
    if (existing_meta_array_count == 0) {
      http_response->status = 500;
      s_set(&http_response->body, "Existing meta data array must not be empty");
      return 1;
    }

    // add requested index items to the existing index meta data
    for (size_t i = 0; i < request_body_array_count; i++) {
      int already_exists = 0;
      const char* request_item =
          json_array_get_string(request_body_json_array, i);

      for (size_t j = 0; j < existing_meta_array_count; j++) {
        const char* existing_item =
            json_array_get_string(existing_meta_json_array, j);

        if (strcmp(request_item, existing_item) == 0) {
          already_exists = 1;
        }
      }
      if (already_exists) continue;
      // append the array item into the existing meta_json_array
      JSON_Status append_status =
          json_array_append_string(existing_meta_json_array, request_item);
      if (append_status != JSONSuccess) {
        http_response->status = 500;
        s_set(&http_response->body,
              "Failed to append new item to existing meta data");
        return 1;
      }
    }

    updated_json_value = json_value_deep_copy(existing_meta_json_value);
  }

  // convert the updated json array to a string
  char* updated_json = json_serialize_to_string(updated_json_value);
  if (updated_json == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to serialize updated JSON data");
    return 1;
  }

  int save_status = save_string(http_response, updated_json, index_meta_path,
                                "\"Index meta inserted successfully\"",
                                "\"Index meta updated successfully\"",
                                "Failed to save data to document");
  if (save_status == -1) return 1;
  return 0;
}