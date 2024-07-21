#include "index_upsert.h"

#define MAX_PATH_LENGTH 1024

int handle_request_index_upsert(HttpRequest* http_request,
                                HttpResponse* http_response) {
  char* requiredParams[] = {"db"};
  QueryParams queries = validate_queries(http_request, requiredParams, 1);
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

  const char* schema_path = derive_path(2, "schema", queries.db);
  char* schema_content =
      get_file_content(http_response, schema_path, "Schema file not found",
                       "Failed to read schema file");
  const JSON_Value* schema_json_value =
      get_json_value(http_response, schema_content, "Schema file not found");
  const JSON_Object* schema_object = json_value_get_object(schema_json_value);
  if (schema_object == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to get schema object");
    return 1;
  }

  for (size_t i = 0; i < request_array_with_count.count; i++) {
    const char* request_meta_string =
        json_array_get_string(request_array_with_count.array, i);
    if (request_meta_string == NULL) {
      http_response->status = 400;
      s_set(&http_response->body,
            "Request body array must contain only strings, failed to parse "
            "string from array.");
      return 1;
    }

    if (json_object_dothas_value(schema_object, request_meta_string) == 0) {
      http_response->status = 400;
      s_set(&http_response->body,
            "Request body array must contain only database document keys that "
            "are present in the schema.");
      return 1;
    }
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
    // Index meta file already exists, lets gooooooo!
    JSON_Array_With_Count existing_meta_array_with_count;
    if (get_json_array_with_count(http_response, existing_meta,
                                  &existing_meta_array_with_count,
                                  "existing index meta") != 0) {
      return 1;
    }

    // add requested index meta items to the existing index meta data
    int insert_new_meta_status =
        insert_new_meta(http_response, request_array_with_count,
                        existing_meta_array_with_count);
    if (insert_new_meta_status != 0) return 1;

    updated_json_value =
        json_value_deep_copy(existing_meta_array_with_count.json_value);
    if (updated_json_value == NULL) {
      http_response->status = 500;
      s_set(&http_response->body, "Failed to copy existing meta JSON data");
      return 1;
    }
  }

  // now actually index the new files
  // get a list of all the files in the db_path directory
  const char* db_path = derive_path(2, "db", queries.db);
  int count;
  char** filenames = get_filenames(db_path, &count);

  if (filenames != NULL) {
    for (size_t i = 0; i < request_array_with_count.count; i++) {
      for (int j = 0; j < count; j++) {
        const char* file_path = derive_path(3, "db", queries.db, filenames[j]);
        if (file_path == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to derive file path to documents to be indexed");
          return 1;
        }
        const char* request_meta_string =
            json_array_get_string(request_array_with_count.array, i);
        if (request_meta_string == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to parse string from request array");
          return 1;
        }

        // get the value we want to index from the database document
        const char* document_dot_value_as_string =
            get_document_dot_value_as_string(http_response, file_path,
                                             request_meta_string);
        if (document_dot_value_as_string == NULL) {
          free_filenames(filenames, count);
          return 1;
        }

        // store that document's "id" in a file named "value"
        // in the directory named "key" in that dbs index directory
        const char* index_path =
            derive_path(4, "index", queries.db, request_meta_string,
                        document_dot_value_as_string);
        if (index_path == NULL) {
          http_response->status = 500;
          s_set(&http_response->body, "Failed to derive index path");
          return 1;
        }

        // performance improvement: check if this index document has been seen
        // already in this request. If so, skip getting the content again here
        char* index_file_content =
            get_file_content(http_response, index_path, "",
                             "Failed to read existing index file");

        if (index_file_content == NULL) {
          // doc doesn't exist yet, create it
          if (http_response->status == 404) {
            // wrap the value in an array
            const JSON_Value* index_json_value = json_value_init_array();
            if (index_json_value == NULL) {
              http_response->status = 500;
              s_set(&http_response->body,
                    "Failed to initialize index JSON data");
              return 1;
            }
            JSON_Array* index_json_array =
                json_value_get_array(index_json_value);
            if (index_json_array == NULL) {
              http_response->status = 500;
              s_set(&http_response->body, "Failed to get index JSON array");
              return 1;
            }
            JSON_Status append_status =
                json_array_append_string(index_json_array, filenames[j]);
            if (append_status != JSONSuccess) {
              http_response->status = 500;
              s_set(&http_response->body,
                    "Failed to append new item to index data");
              return 1;
            }

            // convert the updated json array to a string
            const char* updated_json =
                json_serialize_to_string(index_json_value);

            int save_status =
                save_string(http_response, updated_json, index_path,
                            "\"Documents indexed successfully\"",
                            "\"Documents indexed successfully\"",
                            "Failed to save data to index");
            if (save_status == -1) {
              free_filenames(filenames, count);
              return 1;
            }
            continue;
          } else {
            return 1;
          }
        }

        // doc already exists, update it
        const JSON_Value* index_json_value =
            get_json_value(http_response, index_file_content,
                           "Failed to parse existing index JSON data");
        if (index_json_value == NULL) {
          free_filenames(filenames, count);
          return 1;
        }

        JSON_Array* index_json_array = json_value_get_array(index_json_value);
        if (index_json_array == NULL) {
          http_response->status = 500;
          s_set(&http_response->body, "Index data must be an array");
          return 1;
        }

        // check if the document is already indexed
        int already_indexed = 0;
        for (size_t k = 0; k < json_array_get_count(index_json_array); k++) {
          const char* indexed_filename =
              json_array_get_string(index_json_array, k);
          if (strcmp(indexed_filename, filenames[j]) == 0) {
            already_indexed = 1;
            break;
          }
        }

        if (already_indexed) {
          continue;
        }

        JSON_Status append_status =
            json_array_append_string(index_json_array, filenames[j]);
        if (append_status != JSONSuccess) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to append new item to index data");
          return 1;
        }

        // convert the updated json array to a string
        const char* updated_json = json_serialize_to_string(index_json_value);

        int save_status = save_string(http_response, updated_json, index_path,
                                      "\"Documents indexed successfully\"",
                                      "\"Documents indexed successfully\"",
                                      "Failed to save data to index");
        if (save_status == -1) {
          free_filenames(filenames, count);
          return 1;
        }
      }
    }
    free_filenames(filenames, count);
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
  http_response->status = 200;
  return 0;
}