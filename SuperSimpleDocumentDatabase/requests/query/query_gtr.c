#include "query_gtr.h"

// compare function for qsort
int compare_ints(const void* a, const void* b) { return (*(int*)a - *(int*)b); }

int handle_query_gtr(HttpRequest* http_request, HttpResponse* http_response,
                     QueryParams* queries) {
  char* index_path = derive_path(3, "index", queries->db, queries->key);

  // check if queries.value can be converted to a number
  int query_value = 0;
  if (sscanf(queries->value, "%d", &query_value) != 1) {
    http_response->status = 400;
    s_set(&http_response->body,
          "Invalid value, must be a number for gtr query");
    free(index_path);
    return 1;
  }

  // check that the schema value being checked via queries.key is a number
  char* schema_path = derive_path(2, "schema", queries->db);
  const JSON_Value* schema = json_parse_file(schema_path);
  const JSON_Object* schema_obj = json_value_get_object(schema);
  const JSON_Value* schema_value =
      json_object_get_value(schema_obj, queries->key);
  if (schema_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid schema, key does not exist");
    free(index_path);
    free(schema_path);
    return 1;
  }
  if (json_value_get_type(schema_value) != JSONNumber) {
    http_response->status = 400;
    s_set(&http_response->body,
          "Invalid schema, requested key is not a number. Cannot perform gtr "
          "query on a non-number.");
    free(index_path);
    free(schema_path);
    return 1;
  }
  free(schema_path);

  // get filenames
  int filenames_count = 0;
  char** filenames = get_filenames(index_path, &filenames_count);

  // convert file names to numbers and sort
  int* numbers = (int*)malloc(filenames_count * sizeof(int));

  for (int i = 0; i < filenames_count; i++) {
    if (sscanf(filenames[i], "%d", &numbers[i]) != 1) {
      http_response->status = 500;
      s_set(&http_response->body, "Failed to convert filename to number");
      free(index_path);
      free(filenames);
      free(numbers);
      return 1;
    }
  }

  // filter numbers
  int* filtered_numbers = (int*)malloc(filenames_count * sizeof(int));
  int filtered_count = 0;
  for (int i = 0; i < filenames_count; i++) {
    if (numbers[i] > query_value) {
      filtered_numbers[filtered_count] = numbers[i];
      filtered_count++;
    }
  }

  // sort filtered numbers
  qsort(filtered_numbers, filtered_count, sizeof(int), compare_ints);

  // convert filtered+sorted numbers to a filenames_to_get_array
  char** filenames_to_get = (char**)malloc(filtered_count * sizeof(char*));
  for (int i = 0; i < filtered_count; i++) {
    filenames_to_get[i] = (char*)malloc(24 * sizeof(char));
    snprintf(filenames_to_get[i], 24, "%d", filtered_numbers[i]);
    printf("Filename to get: %s\n", filenames_to_get[i]);
  }

  // get documents
  JSON_Value* documents = json_value_init_array();
  JSON_Array* documents_array = json_value_get_array(documents);
  for (int i = 0; i < filtered_count; i++) {
    char* document_path =
        derive_path(4, "index", queries->db, queries->key, filenames_to_get[i]);
    JSON_Value* document = json_parse_file(document_path);
    json_array_append_value(documents_array, document);
    free(document_path);
  }

  // set response
  http_response->status = 200;
  s_set(&http_response->body, json_serialize_to_string(documents));
  free(index_path);
  free(filenames);
  free(numbers);
  return 0;
}