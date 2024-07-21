#include "compile_entire_doc.h"

int compile_doc_change(HttpResponse* http_response, HttpRequest* http_request,
                       char* db_path, QueryParams queries,
                       char* schema_file_content,
                       const JSON_Value* request_json_value) {
  JSON_Value* schema_json =
      json_parse_string_with_comments(schema_file_content);
  if (schema_json == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Unknown schema error - please check the schema: \n%s\n",
              schema_file_content);
    return 1;
  }
  if (json_validate(schema_json, request_json_value) != JSONSuccess) {
    http_response->status = 400;
    s_compile(&http_response->body, "Invalid schema - Should be:\n%s\n",
              schema_file_content);
    return 1;
  }
  if (json_validate(request_json_value, schema_json) != JSONSuccess) {
    http_response->status = 400;
    s_compile(&http_response->body, "Invalid schema - Should be:\n%s\n",
              schema_file_content);
    return 1;
  }

  char* updated_json = json_serialize_to_string_pretty(request_json_value);
  s_set(&http_request->body, updated_json);
  json_free_serialized_string(updated_json);
  return 0;
}