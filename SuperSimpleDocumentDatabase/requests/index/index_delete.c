#include "index_delete.h"

#define MAX_PATH_LENGTH 1024

int handle_request_index_delete(sdb_http_request_t* http_request,
                                sdb_http_response_t* http_response) {
  const char* params[] = {"db"};
  sdb_query_params_t queries =
      validate_and_parse_queries(http_request, params, 1);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  const char* index_meta_path = derive_path(3, "index", queries.db, "_meta");

  JSON_Array_With_Count request_array_with_count;
  if (get_json_array_with_count(http_response, http_request->body.value,
                                &request_array_with_count, "request") != 0) {
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
  const JSON_Value_With_Removed_Items* updated_json_value_with_removed_items =
      removed_meta(http_response, request_array_with_count, existing_meta);

  // delete the index files and directory
  for (int i = 0;
       i < updated_json_value_with_removed_items->removed_items_count; i++) {
    printf("removed item: %s\n",
           updated_json_value_with_removed_items->removed_items[i]);
    char* removed_item_path =
        derive_path(3, "index", queries.db,
                    updated_json_value_with_removed_items->removed_items[i]);
    if (removed_item_path == NULL) {
      free_removed_items(
          updated_json_value_with_removed_items->removed_items,
          updated_json_value_with_removed_items->removed_items_count);
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