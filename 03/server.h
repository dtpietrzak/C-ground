#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "server.h"

#define PORT 7777
#define MAX_REQ_METHOD_SIZE 16
#define MAX_REQ_PATH_SIZE 256
#define MAX_REQ_HEADERS 12
#define MAX_REQ_HEADER_SIZE 256
#define MAX_REQ_BODY_SIZE 1024

typedef struct {
  char method[MAX_REQ_METHOD_SIZE];
  char path[MAX_REQ_PATH_SIZE];
  char headers[MAX_REQ_HEADERS][MAX_REQ_HEADER_SIZE];
  int num_headers;
  char body[MAX_REQ_BODY_SIZE];
} HttpRequest;

typedef const char *(*RequestHandler)(const HttpRequest *);

char *create_http_header(int status_code, const char *content_type,
                         int content_length);

void parse_http_request(const char *request_str, HttpRequest *request);

const char *process_request(const char *request_str);

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf);

void on_connection(uv_stream_t *server, int status);

int start_server();

#endif  // HTTP_SERVER_H