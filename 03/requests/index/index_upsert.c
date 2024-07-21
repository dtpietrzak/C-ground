#include "index_upsert.h"

#define MAX_PATH_LENGTH 1024

int handle_request_index_upsert(HttpRequest* http_request,
                                HttpResponse* http_response) {
  char* requiredParams[] = {"db", "key"};
  QueryParams queries = validate_queries(http_request, requiredParams, 2);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  const char* db_path = derive_path(2, "db", queries.db);
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
  if (existing_meta == NULL) {
    if (http_response->status != 404) {
      return 1;
    }
    // Index meta file doesn't exist yet, no need to merge
    // just continue to saving the new index meta data
    updated_json_value =
        json_value_deep_copy(request_array_with_count.json_value);
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

    // add requested index items to the existing index meta data
    for (size_t i = 0; i < request_array_with_count.count; i++) {
      int already_exists = 0;
      const char* request_item =
          json_array_get_string(request_array_with_count.array, i);
      if (request_item == NULL) {
        http_response->status = 400;
        s_set(&http_response->body,
              "Request body array must contain only strings, failed to parse "
              "string from array.");
        return 1;
      }

      for (size_t j = 0; j < existing_meta_array_count; j++) {
        const char* existing_item =
            json_array_get_string(existing_meta_json_array, j);
        if (existing_item == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Existing meta data array must contain only strings, failed to "
                "parse string from array.");
          return 1;
        }

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

    // actually index the new files
    // get a list of all the files in the db_path directory
    int count;
    char** filenames = get_filenames(db_path, &count);

    if (filenames != NULL) {
      for (int i = 0; i < count; i++) {
        const char* file_path = derive_path(3, "db", queries.db, filenames[i]);
        if (file_path == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to derive file path to documents to be indexed");
          free_filenames(filenames, count);
          return 1;
        }

        char* file_content =
            get_file_content(http_response, file_path, "Document not found",
                             "Failed to read a document to be indexed");
        if (file_content == NULL) {
          free_filenames(filenames, count);
          return 1;
        }

        JSON_Value* file_json_value =
            json_parse_string_with_comments(file_content);
        if (file_json_value == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to parse document JSON data to be indexed");
          free(file_content);
          free_filenames(filenames, count);
          return 1;
        }

        const JSON_Object* file_json_object =
            json_value_get_object(file_json_value);
        if (file_json_object == NULL) {
          http_response->status = 500;
          s_set(&http_response->body, "Document JSON data must be an object");
          json_value_free(file_json_value);
          free(file_content);
          free_filenames(filenames, count);
          return 1;
        }

        // index the document
        const char* index_value =
            json_object_get_string(file_json_object, queries.key);
        if (index_value == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to get index value from document");
          json_value_free(file_json_value);
          free(file_content);
          return 1;
        }

        const char* index_path =
            derive_path(4, "index", queries.db, queries.key, index_value);

        save_string(http_response, filenames[i], index_path,
                    "\"Documents indexed successfully\"",
                    "\"Documents indexed successfully\"",
                    "Failed to save data to index");
        http_response->status = 200;
      }

      free_filenames(filenames, count);
    }

    updated_json_value = json_value_deep_copy(existing_meta_json_value);
    if (updated_json_value == NULL) {
      http_response->status = 500;
      s_set(&http_response->body, "Failed to copy existing meta JSON data");
      return 1;
    }
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