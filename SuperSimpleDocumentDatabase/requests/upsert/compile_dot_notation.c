#include "compile_dot_notation.h"

int compile_dot_notation_change(sdb_http_response_t* http_response,
                                sdb_http_request_t* http_request, char* db_path,
                                sdb_query_params_t queries,
                                char* schema_file_content,
                                JSON_Value* request_json_value) {
  // Check if the file exists
  sdb_stater_t* stater_doc_exists = calloc(1, sizeof(sdb_stater_t));
  stater_doc_exists->error_body = "Document does not exist";
  stater_doc_exists->error_status = 404;
  if (!fs_file_access(http_response, db_path, stater_doc_exists, F_OK)) {
    return 1;
  }

  // Check if the file is readable
  sdb_stater_t* stater_read_access = calloc(1, sizeof(sdb_stater_t));
  stater_read_access->error_body = "Document does not have read permissions";
  stater_read_access->error_status = 500;
  if (!fs_file_access(http_response, db_path, stater_read_access, R_OK)) {
    return 1;
  }

  // remove the before the first dot and the first dot itself
  char* json_id = strchr(queries.id, '.') + 1;

  JSON_Value* schema_json =
      json_parse_string_with_comments(schema_file_content);
  if (schema_json == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Unknown schema error - please check the schema: \n%s\n",
              schema_file_content);
    return 1;
  }
  JSON_Object* schema_object = json_value_get_object(schema_json);
  if (schema_object == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Unknown schema error - please check the schema: \n%s\n",
              schema_file_content);
    return 1;
  }
  JSON_Value* schema_value = json_object_dotget_value(schema_object, json_id);

  // need to get this validation working
  if (json_value_get_type(schema_value) !=
      json_value_get_type(request_json_value)) {
    http_response->status = 400;
    s_compile(&http_response->body, "Invalid schema - Should be:\n%s\n",
              schema_file_content);
    return 1;
  }

  // Read file content into string
  char* current_document_file_content = read_file_to_string(db_path);
  if (current_document_file_content == NULL) {
    // 500 here even in the case of the file not existing
    // because we're checking that above
    http_response->status = 500;
    s_compile(&http_response->body,
              "Failed to read data from the requested document: %s", db_path);
    return 1;
  }

  JSON_Value* document_json =
      json_parse_string_with_comments(current_document_file_content);
  if (document_json == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body, "Failed to parse JSON");
    return 1;
  }
  JSON_Object* document_json_object = json_value_get_object(document_json);
  if (document_json_object == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body, "Failed to parse JSON");
    return 1;
  }

  JSON_Value* document_value =
      json_object_dotget_value(document_json_object, json_id);
  if (document_value == NULL) {
    http_response->status = 404;
    s_compile(&http_response->body, "id not found in document: %s", queries.id);
    return 1;
  }

  json_object_dotset_value(document_json_object, json_id, request_json_value);
  char* updated_json = json_serialize_to_string_pretty(document_json);
  s_set(&http_request->body, updated_json);
  json_free_serialized_string(updated_json);
  return 0;
}