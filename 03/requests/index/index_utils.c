#include "index_utils.h"

int get_json_array_with_count(HttpResponse* http_response,
                              char* string_to_parse,
                              JSON_Array_With_Count* json_array_with_count,
                              char* error_identity) {
  SString error_message;
  s_init(&error_message, "", 1024);
  s_compile(&error_message, "Failed to parse %s JSON data", error_identity);

  json_array_with_count->json_value =
      get_json_value(http_response, string_to_parse, error_message.value);
  if (json_array_with_count->json_value == NULL) return 1;

  json_array_with_count->array =
      json_value_get_array(json_array_with_count->json_value);
  if (json_array_with_count->array == NULL) {
    http_response->status = 400;
    s_compile(&http_response->body, "%s must be an array", error_identity);
    return 1;
  }

  json_array_with_count->count =
      json_array_get_count(json_array_with_count->array);

  return 0;
}

int insert_new_meta(HttpResponse* http_response,
                    JSON_Array_With_Count request_array_with_count,
                    JSON_Array_With_Count existing_meta_array_with_count) {
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

    for (size_t j = 0; j < existing_meta_array_with_count.count; j++) {
      const char* existing_item =
          json_array_get_string(existing_meta_array_with_count.array, j);
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
    JSON_Status append_status = json_array_append_string(
        existing_meta_array_with_count.array, request_item);
    if (append_status != JSONSuccess) {
      http_response->status = 500;
      s_set(&http_response->body,
            "Failed to append new item to existing meta data");
      return 1;
    }
  }
  return 0;
}

const char* get_document_dot_value_as_string(HttpResponse* http_response,
                                             const char* file_path,
                                             const char* key) {
  char* file_content =
      get_file_content(http_response, file_path, "Document not found",
                       "Failed to read a document to be indexed");
  if (file_content == NULL) {
    return NULL;
  }

  JSON_Value* file_json_value = json_parse_string_with_comments(file_content);
  if (file_json_value == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Failed to parse document JSON data to be indexed - %s key: %s",
              file_path, key);
    free(file_content);
    return NULL;
  }

  const JSON_Object* file_json_object = json_value_get_object(file_json_value);
  if (file_json_object == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, "Document JSON data must be an object");
    json_value_free(file_json_value);
    free(file_content);
    return NULL;
  }

  // index the document
  const char* document_dot_value =
      json_object_dotget_string(file_json_object, key);
  if (document_dot_value == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Failed to get index value from document by key: '%s' - document "
              "value for this key must be a string",
              key);
    json_value_free(file_json_value);
    free(file_content);
    return NULL;
  }

  return document_dot_value;
}