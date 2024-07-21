#include "utils.h"

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