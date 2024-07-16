#include "upsert.h"

#define MAX_PATH_LENGTH 1024

char* get_schema_file_content(HttpResponse* http_response, char* schema_path,
                              char* db_path, QueryParams queries) {
  // Check if the file exists
  char* schema_file_access_issue = check_file_access(schema_path, 1);
  if (schema_file_access_issue != NULL) {
    if (strcmp(schema_file_access_issue, "Document does not exist") == 0) {
      http_response->status = 404;
    } else {
      http_response->status = 500;
    }
    s_compile(&http_response->body,
              "Failed to read schema data for the "
              "requested document: %s - use POST /schema?db=%s&key=%s with an "
              "example of your valid JSON schema and try again.",
              db_path, queries.db, queries.key);
    return NULL;
  }

  char* schema_file = read_file_to_string(schema_path);
  if (schema_file == NULL) {
    http_response->status = 500;
    s_compile(&http_response->body,
              "Failed to read schema data for the "
              "requested document: %s - use POST /schema?db=%s&key=%s with an "
              "example of your valid JSON schema and try again.",
              db_path, queries.db, queries.key);
    return NULL;
  }

  return schema_file;
}

JSON_Value* get_json_value(HttpResponse* http_response, char* string_to_parse) {
  JSON_Value* request_json_value =
      json_parse_string_with_comments(string_to_parse);
  if (request_json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid JSON");
  }

  return request_json_value;
}

int handle_request_upsert(HttpRequest* http_request,
                          HttpResponse* http_response) {
  char* requiredParams[] = {"key", "db"};
  QueryParams queries = validate_queries(http_request, requiredParams, 2);
  if (queries.invalid != NULL) {
    http_response->status = 400;
    s_set(&http_response->body, queries.invalid);
    return 1;
  }

  char* db_path = derive_path_to_item("db", queries.db, queries.key);
  char* schema_path = derive_path_to_location("schema", queries.db);

  JSON_Value* request_json_value =
      json_parse_string_with_comments(http_request->body.value);
  if (request_json_value == NULL) {
    http_response->status = 400;
    s_set(&http_response->body, "Invalid JSON");
    return 1;
  }

  char* schema_file_content =
      get_schema_file_content(http_response, schema_path, db_path, queries);
  if (schema_file_content == NULL) return 1;

  if (contains_periods(queries.key)) {
    // Check if the file exists
    char* file_access_issue = check_file_access(db_path, 1);
    if (file_access_issue != NULL) {
      if (strcmp(file_access_issue, "Document does not exist") == 0) {
        http_response->status = 404;
      } else {
        http_response->status = 500;
      }
      s_compile(&http_response->body, "\"%s: %s\"", file_access_issue, db_path);
      return 1;
    }

    // remove the before the first dot and the first dot itself
    char* json_key = strchr(queries.key, '.') + 1;

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
    JSON_Value* schema_value =
        json_object_dotget_value(schema_object, json_key);

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
        json_object_dotget_value(document_json_object, json_key);
    if (document_value == NULL) {
      http_response->status = 404;
      s_compile(&http_response->body, "Key not found in document: %s",
                queries.key);
      return 1;
    }

    json_object_dotset_value(document_json_object, json_key,
                             request_json_value);
    char* updated_json = json_serialize_to_string_pretty(document_json);

    printf("updated_json: %s\n", updated_json);

    s_set(&http_request->body, updated_json);
  } else {
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

    char* updated_json = json_serialize_to_string_pretty(request_json_value);
    s_set(&http_request->body, updated_json);
    json_free_serialized_string(updated_json);
  }

  json_value_free(request_json_value);

  // -1 error
  // 0 had to create directory
  // 1 directory already existed
  int status = save_string_to_file(http_request->body.value, db_path);
  switch (status) {
    case -1: {
      // Check if the file exists
      char* file_access_issue = check_file_access(db_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
                  db_path);
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "Failed to save data to document: %s",
                db_path);
      return 1;
    }
    case 0:
      http_response->status = 201;
      s_compile(&http_response->body, "\"Document inserted successfully: %s\"",
                db_path);
      return 0;
    case 1:
      http_response->status = 204;
      s_compile(&http_response->body, "\"Document updated successfully: %s\"",
                db_path);
      return 0;
    default: {
      // Check if the file exists
      char* file_access_issue = check_file_access(db_path, -2);
      if (file_access_issue != NULL) {
        http_response->status = 500;
        s_compile(&http_response->body, "\"%s: %s\"", file_access_issue,
                  db_path);
        return 1;
      }
      http_response->status = 500;
      s_compile(&http_response->body, "An unknown error occurred: %s", db_path);
      return 1;
    }
  }
}
