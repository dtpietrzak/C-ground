#include "request_operations.h"

// Get the content of a file and return it as a string
// If the file does not exist, set the response status to 404
// and set the response body to the error message
// If there is an issue reading the file, set the response status to 500
// and set the response body to the error message
//
// Returns the file content if successful, NULL if not
char* get_file_content(sdb_http_response_t* http_response,
                       const char* relative_path, const char* error_message_404,
                       const char* error_message_500) {
  // Check if the file exists
  sdb_stater_t* stater_doc_exists = calloc(1, sizeof(sdb_stater_t));
  stater_doc_exists->error_body = "Requested document does not exist";
  stater_doc_exists->error_status = 404;
  if (!fs_file_access(http_response, relative_path, stater_doc_exists,
                           F_OK)) {
    return NULL;
  }

  // Check if the file is readable
  sdb_stater_t* stater_read_access = calloc(1, sizeof(sdb_stater_t));
  stater_read_access->error_body =
      "Requested document does not have read permissions";
  stater_read_access->error_status = 500;
  if (!fs_file_access(http_response, relative_path, stater_read_access,
                           R_OK)) {
    return NULL;
  }

  char* schema_file = read_file_to_string(relative_path);
  if (schema_file == NULL) {
    http_response->status = 500;
    s_set(&http_response->body, error_message_500);
    return NULL;
  }

  return schema_file;
}

// Parse the JSON string and return the JSON_Value
// If the JSON string is invalid, set the response status to 400
// and set the response body to the error message
//
// Returns the JSON_Value if successful, NULL if not
JSON_Value* get_json_value(sdb_http_response_t* http_response,
                           char* string_to_parse, char* error_message) {
  JSON_Value* request_json_value =
      json_parse_string_with_comments(string_to_parse);
  if (request_json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, error_message);
  }

  return request_json_value;
}

// Save a string to a file
// If the file does not exist, set the response status to 500
// and set the response body to the error message
// If the file is new and saved successfully, set the response status to 201
// and set the response body to the success message
// If the file is updated and saved successfully, set the response status to 204
// and set the response body to the success message
//
// Returns 0 if the file is new and saved successfully
// Returns 1 if the file is updated and saved successfully
// Returns -1 if there is an issue saving the file
int save_string(sdb_http_response_t* http_response, const char* string_to_save,
                const char* relative_path, const char* success_message_201,
                const char* success_message_204, char* error_message_500) {
  int status = save_string_to_file(string_to_save, relative_path);
  switch (status) {
    case -1: {
      // Check for file write access
      sdb_stater_t* stater_write_access = calloc(1, sizeof(sdb_stater_t));
      strcat(error_message_500, " - Document does not have write permissions");
      stater_write_access->error_body = error_message_500;
      stater_write_access->error_status = 500;
      if (!fs_file_access(http_response, relative_path,
                               stater_write_access, W_OK)) {
        return 1;
      }
      http_response->status = 500;
      s_set(&http_response->body, error_message_500);
      return -1;
    }
    case 0:
      http_response->status = 201;
      s_set(&http_response->body, success_message_201);
      return 0;
    case 1:
      http_response->status = 204;
      s_set(&http_response->body, success_message_204);
      return 1;
    default: {
      // Check for file write access
      sdb_stater_t* stater_write_access = calloc(1, sizeof(sdb_stater_t));
      strcat(error_message_500, " - Document does not have write permissions");
      stater_write_access->error_body = error_message_500;
      stater_write_access->error_status = 500;
      if (!fs_file_access(http_response, relative_path,
                               stater_write_access, W_OK)) {
        return 1;
      }
      http_response->status = 500;
      s_set(&http_response->body, "An unknown error occurred");
      return -1;
    }
  }
}