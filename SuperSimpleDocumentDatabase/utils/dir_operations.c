#include "dir_operations.h"

void free_filenames(char** filenames, int count) {
  for (int i = 0; i < count; i++) {
    free(filenames[i]);
  }
  free(filenames);
}

char** get_filenames(const char* dir_path, int* count) {
  DIR* dir = opendir(dir_path);
  if (dir == NULL) {
    perror("opendir");
    return NULL;
  }

  const struct dirent* entry;
  int capacity = 10;  // Initial capacity for filenames array
  char** filenames = malloc(capacity * sizeof(char*));
  *count = 0;

  while ((entry = readdir(dir)) != NULL) {
    // Skip '.' and '..'
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Resize the filenames array if needed
    if (*count >= capacity) {
      capacity *= 2;
      char** temp = realloc(filenames, capacity * sizeof(char*));
      if (temp == NULL) {
        free_filenames(filenames, *count);
        perror("realloc");
        return NULL;
      }
    }

    // Allocate memory for the filename and copy it
    filenames[*count] = malloc(strlen(entry->d_name) + 1);
    strcpy(filenames[*count], entry->d_name);
    (*count)++;
  }

  closedir(dir);
  return filenames;
}

int remove_directory(const char* dir_path) {
  if (dir_path == NULL) {
    return 1;
  }

  printf("Removing directory: %s\n", dir_path);

  int count;
  char** filenames = get_filenames(dir_path, &count);

  if (count == 0 || filenames == NULL) {
    perror("rmdir1");
    int removed_status = remove(dir_path);
    if (removed_status != 0) {
      perror("rmdir2");
      return 1;
    }
    return 0;
  }

  for (int i = 0; i < count; i++) {
    const char* file_name = strdup(filenames[i]);
    if (file_name == NULL) {
      free_filenames(filenames, count);
      return 1;
    }
    int dir_length = strlen(dir_path);
    int file_length = strlen(file_name);
    int path_length = dir_length + file_length + 2;

    // FAILING HERE?
    char* file_path = malloc(path_length);
    if (file_path == NULL) {
      free_filenames(filenames, count);
      return 1;
    }

    file_path[0] = '\0';
    snprintf(file_path, path_length, "%s/%s", dir_path, file_name);
    if (file_path == NULL) {
      free(file_path);
      free_filenames(filenames, count);
      return 1;
    }

    int remove_status = remove(file_path);
    if (remove_status != 0) {
      perror("remove");
      free(file_path);
      free_filenames(filenames, count);
      return 1;
    }
    free(file_path);
  }

  int remove_status = remove(dir_path);
  if (remove_status != 0) {
    perror("remove");
    free_filenames(filenames, count);
    return 1;
  }

  free_filenames(filenames, count);

  return 0;
}