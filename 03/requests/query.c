#include "query.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

#define MAX_PATH_LENGTH 1024

char* handle_request_query(HttpRequest* http_request) {
  char *key = NULL, *db = NULL;
  for (int i = 0; i < http_request->num_queries; i++) {
    if (!strcmp(http_request->queries[i][0], "key")) {
      key = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "db")) {
      db = http_request->queries[i][1];
    }
  }
  if (key == NULL) {
    return "400";
  }
  if (contains_invalid_chars(key)) {
    return "400";
  }
  if (db == NULL) {
    return "400";
  }
  if (contains_invalid_chars(db)) {
    return "400";
  }

  char relative_path[MAX_PATH_LENGTH];
#ifdef _WIN32
  snprintf(relative_path, sizeof(relative_path), "dbs\\%s\\%s", db,
           key);  // Windows uses backslashes
#else
  snprintf(relative_path, sizeof(relative_path), "./dbs/%s/%s", db,
           key);  // Unix-like systems use forward slashes
#endif

  // Read file content into string
  char* file_content = read_file_to_string(relative_path);
  if (file_content == NULL) {
    // Handle error
    return "404";
  }

  return file_content;
}