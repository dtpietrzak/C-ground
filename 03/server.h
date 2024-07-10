#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "distributor.h"

#define PORT 7777

char *create_http_header(char *status_code, const char *content_type,
                         int content_length);

void parse_http_request(const char *request_str, HttpRequest *request);

const char *process_request(const char *request_str);

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf);

void on_connection(uv_stream_t *server, int status);

int start_server();

#endif  // SERVER_H