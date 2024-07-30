#include "access.h"

bool fs_file_exists_sync(sdb_http_response_t* http_response,
                         const char* file_path, sdb_stater_t* stater) {
  uv_loop_t* loop = uv_default_loop();
  uv_fs_t req;

  int result = uv_fs_access(loop, &req, file_path, F_OK, NULL);
  bool file_exists = result == 0;

  if (file_exists) {
    http_response->status = stater->success_status;
    if (stater->success_body != NULL) {
      s_set(&http_response->body, stater->success_body);
    }
  } else {
    http_response->status = 404;
    if (stater->error_body != NULL) {
      s_set(&http_response->body, stater->error_body);
    }
  }

  free_stater(stater);
  uv_fs_req_cleanup(&req);
  return file_exists;
}