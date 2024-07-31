#include "access.h"

bool fs_file_load_sync(sdb_http_response_t* http_response, char** out_buffer,
                       const char* file_path, sdb_stater_t* stater, int mode) {
  uv_loop_t* loop = uv_default_loop();
  uv_fs_t open_req;

  int fd = uv_fs_open(loop, &open_req, file_path, 0, mode, NULL);
  if (fd < 0) {
    int error_code = (int)open_req.result;
    if (stater->error_status != 0) {
      http_response->status = stater->error_status;
    }
    if (stater->error_body != NULL) {
      s_compile(&http_response->body, "%s - Failed to open document(%d): %s",
                stater->error_body, error_code, file_path);
    }
    free_stater(stater);
    uv_fs_req_cleanup(&open_req);
    *out_buffer = NULL;
    return false;
  }

  uv_fs_t fstat_req;
  int stat_result = uv_fs_fstat(loop, &fstat_req, fd, NULL);
  if (stat_result < 0) {
    int error_code = (int)fstat_req.result;
    if (stater->error_status != 0) {
      http_response->status = stater->error_status;
    }
    if (stater->error_body != NULL) {
      s_compile(&http_response->body,
                "%s - Failed to get document status(%d): %s",
                stater->error_body, error_code, file_path);
    }
    uv_fs_close(loop, &open_req, fd, NULL);
    free_stater(stater);
    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&fstat_req);
    *out_buffer = NULL;
    return false;
  }

  size_t file_size = ((uv_stat_t*)fstat_req.ptr)->st_size;

  // Allocate a buffer of the appropriate size
  char* buffer = (char*)malloc((file_size + 1) * sizeof(char));
  if (!buffer) {
    if (stater->error_status != 0) {
      http_response->status = stater->error_status;
    }
    if (stater->error_body != NULL) {
      s_compile(&http_response->body,
                "%s - Failed to allocate buffer for document read: %s",
                stater->error_body, file_path);
    }
    uv_fs_close(loop, &open_req, fd, NULL);
    free_stater(stater);
    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&fstat_req);
    *out_buffer = NULL;
    return false;
  }
  buffer[file_size] = '\0';

  uv_fs_t read_req;
  const uv_buf_t iov = uv_buf_init(buffer, file_size);

  int result = uv_fs_read(loop, &read_req, fd, &iov, 1, 0, NULL);
  if (result < 0) {
    int error_code = (int)read_req.result;
    if (stater->error_status != 0) {
      http_response->status = stater->error_status;
    }
    if (stater->error_body != NULL) {
      s_compile(&http_response->body,
                "%s - Failed to read document(%d - %s): %s", stater->error_body,
                error_code, uv_strerror(error_code), file_path);
    }
    uv_fs_close(loop, &open_req, fd, NULL);
    free_stater(stater);
    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&fstat_req);
    uv_fs_req_cleanup(&read_req);
    free(buffer);
    *out_buffer = NULL;
    return false;
  }

  uv_fs_close(loop, &open_req, fd, NULL);
  free_stater(stater);
  uv_fs_req_cleanup(&open_req);
  uv_fs_req_cleanup(&fstat_req);
  uv_fs_req_cleanup(&read_req);
  *out_buffer = buffer;
  return true;
}