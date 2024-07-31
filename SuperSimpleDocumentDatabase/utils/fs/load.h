#ifndef FS_LOAD_H
#define FS_LOAD_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#include "../request_state/request_state.h"

bool fs_file_load(sdb_http_response_t* http_response, char** out_buffer,
                       const char* file_path, sdb_stater_t* stater);

// if / when the time comes, add this function to allow a single file open
// for subsequent reads / writes.
// int fs_file_load_fd(sdb_http_response_t* http_response, char**
// out_buffer,
//                          const char* file_path, sdb_stater_t* stater, int
//                          mode);

#endif  // FS_LOAD_H