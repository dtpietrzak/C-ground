#include "request_state.h"

void free_http_request(sdb_http_request_t *http_request) {
  memset(&http_request->headers, 0, sizeof(http_request->headers));
  memset(&http_request->method, 0, sizeof(http_request->method));
  memset(&http_request->path, 0, sizeof(http_request->path));
  memset(&http_request->queries, 0, sizeof(http_request->queries));
  s_free(&http_request->body);
}

void free_http_response(sdb_http_response_t *http_response) {
  http_response->status = 0;
  s_free(&http_response->body);
}

void free_stater(sdb_stater_t *stater) {
  if (stater != NULL) {
    stater->success_body = NULL;
    stater->error_body = NULL;
    stater->success_status = 0;
    stater->error_status = 0;
    free(stater);
  }
}