#include "index_upsert.h"

#define MAX_PATH_LENGTH 1024

int handle_request_index_upsert(sdb_http_request_t* http_request,
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

  char* existing_meta = get_file_content(http_response, index_meta_path,
                                         "Index meta file not found",
                                         "Failed to read index meta file");
  const JSON_Value* updated_json_value =
      upserted_meta(http_response, request_array_with_count, existing_meta);

  // now actually index the new files
  // get a list of all the files in the db_path directory
  const char* db_path = derive_path(2, "db", queries.db);
  int file_count;
  char** filenames = get_filenames(db_path, &file_count);

  if (filenames != NULL) {
    for (size_t i = 0; i < request_array_with_count.count; i++) {
      for (int j = 0; j < file_count; j++) {
        const char* request_meta_string =
            json_array_get_string(request_array_with_count.array, i);
        if (request_meta_string == NULL) {
          http_response->status = 500;
          s_set(&http_response->body,
                "Failed to parse string from request array");
          return 1;
        }

        int index_a_doc_status = index_a_doc(http_response, queries.db,
                                             filenames[j], request_meta_string);
        if (index_a_doc_status != 0) {
          free_filenames(filenames, file_count);
          return 1;
        }
      }
    }
    free_filenames(filenames, file_count);
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