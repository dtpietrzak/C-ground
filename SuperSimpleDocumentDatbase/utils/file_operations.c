#include "file_operations.h"

#define MAX_PATH_LENGTH 1024

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

char* derive_path(int num_args, ...) {
  // Start with the base path length
  size_t length = strlen(global_setting_path) + 5;  // "data/" + null terminator
  va_list args;
  va_start(args, num_args);

  // Calculate the total length required
  for (int i = 0; i < num_args; i++) {
    const char* arg = va_arg(args, const char*);
    if (arg == NULL) {
      fprintf(stderr, "Null pointer encountered for argument %d\n", i);
      va_end(args);
      return NULL;
    }
    length += strlen(arg) + 1;  // For the arg length and a slash
  }

  // Allocate memory for the resulting path
  char* relative_path = malloc(length);
  if (relative_path == NULL) {
    perror("Failed to allocate memory");
    va_end(args);
    return NULL;
  }

  // Construct the path
  strcpy(relative_path, global_setting_path);
  strcat(relative_path, "data");
  strcat(relative_path, PATH_SEPARATOR);

  va_start(args, num_args);  // Re-initialize args to iterate again
  for (int i = 0; i < num_args; i++) {
    const char* arg = va_arg(args, const char*);
    strcat(relative_path, arg);
    if (i < num_args - 1) {
      strcat(relative_path, PATH_SEPARATOR);
    }
  }

  va_end(args);

  // remove everything after a . in the last / segment of relative_path
  const char* last_slash = strrchr(relative_path, PATH_SEPARATOR[0]);
  if (last_slash != NULL) {
    char* dot = strchr(last_slash, '.');
    if (dot != NULL) {
      *dot = '\0';
    }
  }

  return relative_path;
}

int make_directory(const char* path) {
  char* dir_copy = strdup(path);
  char* last_slash = strrchr(dir_copy, '/');
  int rec_status = 0;

  if (last_slash != NULL) {
    *last_slash = '\0';
    rec_status = make_directory(dir_copy);
    free(dir_copy);

    if (rec_status == -1) {
      return -1;
    }
  }

  int status = mkdir(path, 0777);
  if (status == -1) {
    if (errno == EEXIST) {
      // directory already exists
      return 1;
    } else {
      // failed to create directory
      perror("Failed to create directory");
      return -1;
    }
  }
  // directory had to be created;
  return 0;
}

char* read_file_to_string(const char* relative_file_path) {
  FILE* file = fopen(relative_file_path, "r");
  if (file == NULL) {
    perror("Failed to open file");
    return NULL;
  }

  // Determine the file size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  // Allocate memory for the file content
  char* file_content = (char*)malloc(file_size + 1);  // +1 for null terminator
  if (file_content == NULL) {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  // Read the file into the buffer
  size_t read_size = fread(file_content, 1, file_size, file);
  if (read_size != (size_t)file_size) {
    perror("Failed to read file");
    free(file_content);
    fclose(file);
    return NULL;
  }

  // Null-terminate the string
  file_content[file_size] = '\0';

  // Close the file
  fclose(file);

  return file_content;
}

int save_string_to_file(const char* data_string,
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
  int dir_status = make_directory(dir);
  if (dir_status == -1) {
    return -1;
  }

  // Check if the file already exists
  int file_existed = (access(relative_file_path, F_OK) == 0) ? 1 : 0;

  FILE* file = fopen(relative_file_path, "w");
  if (file == NULL) {
    perror("Failed to open file");
    return -1;
  }
  fprintf(file, "%s", data_string);
  fclose(file);
  // this has a bug rn
  // 0 newly created (create), 1 already exists (update), -1 if failed
  if (dir_status == 1 && file_existed == 1) {
    return 1;
  } else {
    return 0;
  }
}

// levels:
// -3 = file only needs executable
// -2 file only needs writable
// -1 file only needs readable
// 0 = file only needs to exist
// 1 = file needs to exist and be readable
// 2 = file needs to exist, be readable and writable
// 3 = file needs to exist, be readable, writable and executable
//
// returns NULL if no issues
//   else a string describing the issue
char* check_file_access(const char* path, const int8_t level) {
  // Check if the file is executable
  if (level == -3) {
    if (access(path, X_OK) == 0) {
      return NULL;
    } else {
      return "Document is not executable";
    }
  }

  // Check if the file is writable
  if (level == -2) {
    if (access(path, W_OK) == 0) {
      return NULL;
    } else {
      return "Document is not writable";
    }
  }

  // Check if the file is readable
  if (level == -1) {
    if (access(path, R_OK) == 0) {
      return NULL;
    } else {
      return "Document is not readable";
    }
  }

  // Check if the file exists
  if (access(path, F_OK) != 0) {
    return "Document does not exist";
  }

  // Check if the file is readable
  if (access(path, R_OK) != 0 && level >= 1) {
    return "Document is not readable";
  }

  // Check if the file is writable
  if (access(path, W_OK) != 0 && level >= 2) {
    return "Document is not writable";
  }

  // Check if the file is executable
  if (access(path, X_OK) != 0 && level >= 3) {
    return "Document is not executable";
  }

  return NULL;
}