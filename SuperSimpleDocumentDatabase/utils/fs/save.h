#ifndef FS_SAVE_H
#define FS_SAVE_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#include "../request_state/request_state.h"

bool fs_file_save(sdb_http_response_t* http_response, char* buffer_to_save,
                  const char* file_path, sdb_stater_t* stater);

#endif  // FS_SAVE_H