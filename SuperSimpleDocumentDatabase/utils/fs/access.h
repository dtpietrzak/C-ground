#ifndef FS_ACCESS_H
#define FS_ACCESS_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#include "../request_state/request_state.h"

bool fs_file_access(sdb_http_response_t* http_response,
                         const char* file_path, sdb_stater_t* stater, int mode);

#endif  // FS_ACCESS_H