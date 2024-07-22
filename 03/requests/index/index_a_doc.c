#include "index_a_doc.h"

int index_a_doc(HttpResponse* http_response, const char* db_name,
                const char* filename, const char* request_meta_string) {
  const char* file_path = derive_path(3, "db", db_name, filename);
  if (file_path == NULL) {
    http_response->status = 500;
    s_set(&http_response->body,
          "Failed to derive file path to documents to be indexed");
    return 1;
  }

  // get the value we want to index from the database document
  const char* document_dot_value_as_string = get_document_dot_value_as_string(
      http_response, file_path, request_meta_string);
  if (document_dot_value_as_string == NULL) {
    return 1;
  }

  // store that document's "id" in a file named "value"
  // in the directory named "key" in that dbs index directory
  const char* index_path = derive_path(4, "index", db_name, request_meta_string,
                                       document_dot_value_as_string);
  if (index_path == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to derive index path");
    return 1;
  }

  // TODO performance improvement: check if this index document has been
  // seen already in this request. If so, skip getting the content again
  char* index_file_content = get_file_content(
      http_response, index_path, "", "Failed to read existing index file");

  if (index_file_content == NULL) {
    // doc doesn't exist yet, create it
    if (http_response->status == 404) {
      // wrap the value in an array
      const JSON_Value* index_json_value = json_value_init_array();
      if (index_json_value == NULL) {
        http_response->status = 500;
        s_set(&http_response->body, "Failed to initialize index JSON data");
        return 1;
      }
      JSON_Array* index_json_array = json_value_get_array(index_json_value);
      if (index_json_array == NULL) {
        http_response->status = 500;
        s_set(&http_response->body, "Failed to get index JSON array");
        return 1;
      }
      JSON_Status append_status =
          json_array_append_string(index_json_array, filename);
      if (append_status != JSONSuccess) {
        http_response->status = 500;
        s_set(&http_response->body, "Failed to append new item to index data");
        return 1;
      }

      // convert the updated json array to a string
      const char* updated_json = json_serialize_to_string(index_json_value);

      int save_status = save_string(http_response, updated_json, index_path,
                                    "\"Documents indexed successfully\"",
                                    "\"Documents indexed successfully\"",
                                    "Failed to save data to index");
      if (save_status == -1) {
        return 1;
      }
      return 0;
    } else {
      return 1;
    }
  }

  // doc already exists, update it
  const JSON_Value* index_json_value =
      get_json_value(http_response, index_file_content,
                     "Failed to parse existing index JSON data");
  if (index_json_value == NULL) {
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
    const char* indexed_filename = json_array_get_string(index_json_array, k);
    if (strcmp(indexed_filename, filename) == 0) {
      already_indexed = 1;
      break;
    }
  }

  if (already_indexed) {
    return 0;
  }

  JSON_Status append_status =
      json_array_append_string(index_json_array, filename);
  if (append_status != JSONSuccess) {
    http_response->status = 500;
    s_set(&http_response->body, "Failed to append new item to index data");
    return 1;
  }

  // convert the updated json array to a string
  const char* updated_json = json_serialize_to_string(index_json_value);

  int save_status = save_string(http_response, updated_json, index_path,
                                "\"Documents indexed successfully\"",
                                "\"Documents indexed successfully\"",
                                "Failed to save data to index");
  if (save_status == -1) {
    return 1;
  }
  return 0;
}