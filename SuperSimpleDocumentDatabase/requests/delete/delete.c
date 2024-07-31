#include "delete.h"

int handle_request_delete(sdb_http_request_t* http_request,
                          sdb_http_response_t* http_response) {
  const char* params[] = {"id", "db"};

  sdb_query_params_t queries =
      validate_and_parse_queries(http_request, params, 2);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  char* db_path = derive_path(3, "db", queries.db, queries.id);
  if (db_path == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Failed to derive path");
    return 1;
  }

  // Check if the file exists
  sdb_stater_t* stater_doc_exists = calloc(1, sizeof(sdb_stater_t));
  stater_doc_exists->error_body = "Requested document does not exist";
  stater_doc_exists->error_status = 404;
  if (!fs_file_access(http_response, db_path, stater_doc_exists, F_OK)) {
    return 1;
  }

  // Try to delete the file
  if (remove(db_path) == 0) {
    http_response->status = 200;
    s_compile(&http_response->body, "\"Removed %s successfully\"", db_path);
    return 0;
  } else {
    http_response->status = 500;
    s_compile(&http_response->body, "Failed to remove %s", db_path);
    return 1;
  }
}