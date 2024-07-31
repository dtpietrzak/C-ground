#include "doc.h"

#define MAX_PATH_LENGTH 1024

int handle_request_doc(sdb_http_request_t* http_request,
                       sdb_http_response_t* http_response) {
  const char* params[] = {"id", "db"};

  sdb_query_params_t queries =
      validate_and_parse_queries(http_request, params, 2);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  const char* db_path = derive_path(3, "db", queries.db, queries.id);

  // if db_path is an error message
  if (db_path == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Failed to derive path");
    return 1;
  }

  // Check if the file exists
  sdb_stater_t* stater_doc_exists = calloc(1, sizeof(sdb_stater_t));
  stater_doc_exists->error_body = "Requested document does not exist";
  stater_doc_exists->error_status = 404;
  if (!fs_file_access_sync(http_response, db_path, stater_doc_exists, F_OK)) {
    return 1;
  }

  // Check if the file is readable
  sdb_stater_t* stater_read_access = calloc(1, sizeof(sdb_stater_t));
  stater_read_access->error_body =
      "Requested document does not have read permissions";
  stater_read_access->error_status = 500;
  if (!fs_file_access_sync(http_response, db_path, stater_read_access, R_OK)) {
    return 1;
  }

  char* file_content = NULL;
  sdb_stater_t* stater_load = calloc(1, sizeof(sdb_stater_t));
  stater_load->error_body = "Failed to load document";
  stater_load->error_status = 500;
  if (!fs_file_load_sync(http_response, &file_content, db_path, stater_load,
                         O_RDONLY)) {
    return 1;
  }

  if (contains_periods(queries.id)) {
    // remove the part before the first dot and the first dot itself
    const char* json_id = strchr(queries.id, '.') + 1;
    char* json_id_decoded = url_decode(json_id);

    // parse the json to get its value
    const JSON_Value* json = json_parse_string_with_comments(file_content);
    const JSON_Object* json_obj = json_value_get_object(json);
    const JSON_Value* value =
        json_object_dotget_value(json_obj, json_id_decoded);
    free(json_id_decoded);
    if (value == NULL) {
      http_response->status = 404;
      s_compile(&http_response->body, "id not found in document: %s",
                queries.id);
      free(file_content);
      return 1;
    } else {
      http_response->status = 200;
      s_set(&http_response->body, json_serialize_to_string(value));
      free(file_content);
      return 0;
    }
  } else {
    // get the whole object, but filter out any comments
    // from the JSON and minify it (done via parson)
    const JSON_Value* json = json_parse_string_with_comments(file_content);
    if (json == NULL) {
      http_response->status = 500;
      s_compile(&http_response->body, "Failed to parse JSON");
      free(file_content);
      return 1;
    } else {
      http_response->status = 200;
      s_set(&http_response->body, json_serialize_to_string(json));
      free(file_content);
      return 0;
    }
  }
}