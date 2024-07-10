#include "upsert.h"

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

// List of invalid characters for file names
const char* invalid_chars = "\\/:*?\"<>|";

// Function to check if a string contains any invalid characters
bool contains_invalid_chars(const char* str) {
  for (const char* p = str; *p != '\0'; p++) {
    if (strchr(invalid_chars, *p) != NULL) {
      return true;
    }
  }
  return false;
}

int make_directory(const char* path) {
  char* dir_copy = strdup(path);
  char* last_slash = strrchr(dir_copy, '/');
  if (last_slash != NULL) {
    *last_slash = '\0';
    make_directory(dir_copy);
    free(dir_copy);
  }

  int status = mkdir(path, 0777);
  if (status == -1) {
    if (errno == EEXIST) {
      // Directory already exists
      return 0;
    } else {
      // Failed to create directory
      perror("Failed to create directory");
      return -1;
    }
  }
  return 0;
}

void save_string_to_file(const char* data_string,
                         const char* relative_file_path) {
  // Create directory if it doesn't exist
  char path_copy[strlen(relative_file_path) + 1];
  strcpy(path_copy, relative_file_path);
#ifdef _WIN32
  char* dir =
      path_copy;  // Windows version of dirname modifies the string itself
#else
  char* dir = dirname(path_copy);  // POSIX version of dirname
#endif
  make_directory(dir);

  FILE* file = fopen(relative_file_path, "w");
  if (file == NULL) {
    perror("Failed to open file");
    return;
  }
  fprintf(file, "%s", data_string);
  fclose(file);
}

char* handle_request_insert(HttpRequest* http_request) {
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
  snprintf(relative_path, sizeof(relative_path), "dbs\\%s\\%s.txt", db,
           key);  // Windows uses backslashes
#else
  snprintf(relative_path, sizeof(relative_path), "./dbs/%s/%s.txt", db,
           key);  // Unix-like systems use forward slashes
#endif

  save_string_to_file(http_request->body, relative_path);
  return http_request->body;
}