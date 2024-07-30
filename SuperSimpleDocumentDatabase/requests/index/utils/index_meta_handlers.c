#include "index_meta_handlers.h"

// upsert the requested index meta items to the existing index meta data

const JSON_Value* upserted_meta(sdb_http_response_t* http_response,
                                JSON_Array_With_Count request_array_with_count,
                                char* existing_meta) {
  const JSON_Value* updated_json_value = json_value_init_array();
  if (updated_json_value == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to initialize updated JSON data");
    return NULL;
  }

  if (existing_meta == NULL) {
    if (http_response->status != 404) {
      return NULL;
    }
    // Index meta file doesn't exist yet, no need to merge
    // just continue to saving the new index meta data
    updated_json_value =
        json_value_deep_copy(request_array_with_count.json_value);
  } else {
    // Index meta file already exists, lets gooooooo!
    JSON_Array_With_Count existing_meta_array_with_count;
    if (get_json_array_with_count(http_response, existing_meta,
                                  &existing_meta_array_with_count,
                                  "existing index meta") != 0) {
      return NULL;
    }

    // add requested index meta items to the existing index meta data
    int insert_new_meta_status =
        insert_new_meta(http_response, request_array_with_count,
                        existing_meta_array_with_count);
    if (insert_new_meta_status != 0) return NULL;

    updated_json_value =
        json_value_deep_copy(existing_meta_array_with_count.json_value);
    if (updated_json_value == NULL) {
      http_response->status = 500;
      s_set(&http_response->body, "Failed to copy existing meta JSON data");
      return NULL;
    }
  }
  return updated_json_value;
}

void free_removed_items(char** removed_items, int count) {
  if (removed_items == NULL) {
    return;
  }
  for (int i = 0; i < count; i++) {
    free(removed_items[i]);
    removed_items[i] = NULL;
  }
  free(*removed_items);
  *removed_items = NULL;  // Set the main array pointer to NULL
}

//  remove the requested index meta items from the existing index meta data

const JSON_Value_With_Removed_Items* removed_meta(
    sdb_http_response_t* http_response, JSON_Array_With_Count request_array_with_count,
    char* existing_meta) {
  // argument guards start
  if (existing_meta == NULL) return NULL;
  // argument guards end

  // return instantiations start
  JSON_Value_With_Removed_Items* result = NULL;
  JSON_Value* updated_json_value = json_value_init_array();
  char** removed_items = NULL;
  int removed_items_count = 0;
  // returned instantiations end

  if (updated_json_value == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to initialize updated JSON data");
    goto cleanup;
  }

  removed_items = malloc(sizeof(char*));
  if (removed_items == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to allocate memory for removed items");
    goto cleanup;
  }

  JSON_Array_With_Count existing_meta_array_with_count;
  if (get_json_array_with_count(http_response, existing_meta,
                                &existing_meta_array_with_count,
                                "existing index meta") != 0) {
    goto cleanup;
  }

  // add requested index items to the existing index meta data
  for (size_t i = 0; i < request_array_with_count.count; i++) {
    const char* request_item =
        json_array_get_string(request_array_with_count.array, i);
    if (request_item == NULL) {
      http_response->status = 400;
      s_compile(&http_response->body,
                "Request body array must contain only strings, failed to parse "
                "string from array. %d",
                i);
      goto cleanup;
    }

    for (size_t j = existing_meta_array_with_count.count; j > 0; j--) {
      const char* existing_item =
          json_array_get_string(existing_meta_array_with_count.array, j - 1);
      if (existing_item == NULL) {
        http_response->status = 500;
        s_compile(
            &http_response->body,
            "Existing meta data array must contain only strings, failed to "
            "parse string from array. %d",
            j);
        goto cleanup;
      }

      if (strcmp(request_item, existing_item) == 0) {
        // have to copy the removed_items before its removed, lol
        removed_items_count++;
        const char* removed_item = strdup(existing_item);
        if (removed_item == NULL) {
          http_response->status = 500;
          s_set(&http_response->body, "Failed to copy removed item");
          goto cleanup;
        }
        char** temp =
            realloc(removed_items, removed_items_count * sizeof(char*));
        if (temp == NULL) {
          free_removed_items(removed_items, removed_items_count - 1);
          http_response->status = 500;
          s_set(&http_response->body, "Failed to reallocate removed items");
          goto cleanup;
        }
        removed_items = temp;
        removed_items[i] = (char*)removed_item;

        // now remove it
        int remove_status =
            json_array_remove(existing_meta_array_with_count.array, j - 1);
        if (remove_status == JSONFailure) {
          http_response->status = 500;
          s_set(&http_response->body, "Failed to remove item from array");
          goto cleanup;
        }
        existing_meta_array_with_count.count--;
      }
    }
  }

  updated_json_value =
      json_value_deep_copy(existing_meta_array_with_count.json_value);
  if (updated_json_value == NULL) {
    // free_removed_items(removed_items, removed_items_count);
    http_response->status = 500;
    s_set(&http_response->body, "Failed to copy existing meta JSON data");
    goto cleanup;
  }

  if (removed_items_count == 0) {
    http_response->status = 404;
    s_set(&http_response->body, "No items found to remove");
    goto cleanup;
  }

  result = malloc(sizeof(JSON_Value_With_Removed_Items));
  if (result == NULL) {
    http_response->status = 500;
    s_set(
        &http_response->body,
        "Failed to allocate memory for updated JSON value with removed items");
    goto cleanup;
  }

  result->json_value = updated_json_value;
  result->removed_items_count = removed_items_count;
  result->removed_items = removed_items;
  return result;

cleanup:
  free_removed_items(removed_items, removed_items_count);
  return result;
}

int free_json_value_with_removed_items(
    JSON_Value_With_Removed_Items* json_value_with_removed_items) {
  if (json_value_with_removed_items != NULL) {
    if (json_value_with_removed_items->json_value != NULL) {
      json_value_free(json_value_with_removed_items->json_value);
      json_value_with_removed_items->json_value = NULL;
    }
    if (json_value_with_removed_items->removed_items != NULL) {
      free_removed_items(json_value_with_removed_items->removed_items,
                         json_value_with_removed_items->removed_items_count);
      json_value_with_removed_items->removed_items = NULL;
      json_value_with_removed_items->removed_items_count = 0;
    }
    free(json_value_with_removed_items);
  };
  return 0;
}