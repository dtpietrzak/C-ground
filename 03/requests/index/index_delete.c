#include "index_delete.h"

#define MAX_PATH_LENGTH 1024

void free_removed_items(char** removed_items, int count) {
  for (int i = 0; i < count - 1; i++) {
    free(removed_items[i]);
  }
  free(removed_items);
}

int handle_request_index_delete(HttpRequest* http_request,
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
  if (existing_meta == NULL) return 1;

  const JSON_Value* existing_meta_json_value = get_json_value(
      http_response, existing_meta, "Failed to parse existing meta JSON data");
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

  // intantiate a string to hold the removed items
  int removed_items_count = 0;
  char** removed_items = malloc(sizeof(char*));
  if (removed_items == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to allocate memory for removed items");
    return 1;
  }

  // add requested index items to the existing index meta data
  for (size_t i = 0; i < request_body_array_count; i++) {
    const char* request_item =
        json_array_get_string(request_body_json_array, i);
    if (request_item == NULL) {
      http_response->status = 400;
      s_compile(&http_response->body,
                "Request body array must contain only strings, failed to parse "
                "string from array. %d",
                i);
      return 1;
    }

    for (size_t j = existing_meta_array_count; j > 0; j--) {
      const char* existing_item =
          json_array_get_string(existing_meta_json_array, j - 1);
      if (existing_item == NULL) {
        http_response->status = 500;
        s_compile(
            &http_response->body,
            "Existing meta data array must contain only strings, failed to "
            "parse string from array. %d",
            j);
        return 1;
      }

      if (strcmp(request_item, existing_item) == 0) {
        // have to copy the removed_items before its removed, lol
        removed_items_count++;
        const char* removed_item = strdup(existing_item);
        if (removed_item == NULL) {
          http_response->status = 500;
          s_set(&http_response->body, "Failed to copy removed item");
          return 1;
        }
        char** temp =
            realloc(removed_items, removed_items_count * sizeof(char*));
        if (temp == NULL) {
          free_removed_items(removed_items, removed_items_count - 1);
          http_response->status = 500;
          s_set(&http_response->body, "Failed to reallocate removed items");
          return 1;
        }
        removed_items = temp;
        removed_items[i] = (char*)removed_item;

        // now remove it
        int remove_status = json_array_remove(existing_meta_json_array, j - 1);
        if (remove_status == JSONFailure) {
          http_response->status = 500;
          s_set(&http_response->body, "Failed to remove item from array");
          return 1;
        }
        existing_meta_array_count--;
      }
    }
  }

  if (removed_items_count == 0) {
    http_response->status = 404;
    s_set(&http_response->body, "No items found to remove");
    return 1;
  }

  // delete the index files and directory
  for (int i = 0; i < removed_items_count; i++) {
    char* removed_item_path =
        derive_path(3, "index", queries.db, removed_items[i]);

    if (removed_item_path == NULL) {
      free_removed_items(removed_items, removed_items_count);
      http_response->status = 500;
      s_set(&http_response->body, "Failed to derive removed item path");
      return 1;
    }

    int remove_status = remove_directory(removed_item_path);
    if (remove_status == -1) {
      http_response->status = 500;
      s_compile(&http_response->body, "Failed to remove directory %s",
                removed_item_path);
      return 1;
    }
  }

  updated_json_value = json_value_deep_copy(existing_meta_json_value);
  if (updated_json_value == NULL) {
    // free_removed_items(removed_items, removed_items_count);
    http_response->status = 500;
    s_set(&http_response->body, "Failed to copy existing meta JSON data");
    return 1;
  }

  // convert the updated json array to a string
  char* updated_json = json_serialize_to_string(updated_json_value);
  if (updated_json == NULL) {
    // free_removed_items(removed_items, removed_items_count);
    http_response->status = 500;
    s_set(&http_response->body, "Failed to serialize updated JSON data");
    return 1;
  }

  int save_status = save_string(http_response, updated_json, index_meta_path,
                                "\"Index meta removed successfully\"",
                                "\"Index meta removed successfully\"",
                                "Failed to save data to document");
  // free_removed_items(removed_items, removed_items_count);
  if (save_status == -1) return 1;
  http_response->status = 200;
  return 0;
}