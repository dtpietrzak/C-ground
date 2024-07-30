// #include "save.h"

// int save_string_to_file(const char* data_string,
//                         const char* relative_file_path) {

  

//   // Create directory if it doesn't exist
//   char path_copy[strlen(relative_file_path) + 1];
//   strcpy(path_copy, relative_file_path);

// #ifdef _WIN32
//   char* dir =
//       path_copy;  // Windows version of dirname modifies the string itself
// #else
//   char* dir = dirname(path_copy);  // POSIX version of dirname
// #endif
//   int dir_status = make_directory(dir);
//   if (dir_status == -1) {
//     return -1;
//   }

//   // Check if the file already exists
//   int file_existed = (access(relative_file_path, F_OK) == 0) ? 1 : 0;

//   FILE* file = fopen(relative_file_path, "w");
//   if (file == NULL) {
//     perror("Failed to open file");
//     return -1;
//   }
//   fprintf(file, "%s", data_string);
//   fclose(file);
//   // this has a bug rn
//   // 0 newly created (create), 1 already exists (update), -1 if failed
//   if (dir_status == 1 && file_existed == 1) {
//     return 1;
//   } else {
//     return 0;
//   }
// }