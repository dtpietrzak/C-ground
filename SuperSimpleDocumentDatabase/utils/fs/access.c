#include "access.h"

bool fs_file_access_sync(sdb_http_response_t* http_response,
                         const char* file_path, sdb_stater_t* stater,
                         int mode) {
  uv_loop_t* loop = uv_default_loop();
  uv_fs_t req;

  int result = uv_fs_access(loop, &req, file_path, mode, NULL);
  bool file_readable = result == 0;

  if (file_readable) {
    if (stater->success_status != 0) {
      http_response->status = stater->success_status;
    }
    if (stater->success_body != NULL) {
      s_compile(&http_response->body, "%s: %s", stater->success_body,
                file_path);
    }
  } else {
    if (stater->error_status != 0) {
      http_response->status = stater->error_status;
    }
    if (stater->error_body != NULL) {
      s_compile(&http_response->body, "%s: %s", stater->error_body, file_path);
    }
  }

  free_stater(stater);
  uv_fs_req_cleanup(&req);
  return file_readable;
}