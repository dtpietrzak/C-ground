#ifndef SERVER_H
#define SERVER_H

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "distributor.h"
#include "http.h"
#include "utils/sstring.h"
#include "utils/utils.h"

#define MAX_RES_SIZE 65535
#define MAX_REQ_SIZE 65535

void process_request(const char *request_str, SString *response_str);

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf);

void on_connection(uv_stream_t *server, int status);

int start_server(int port);

#endif  // SERVER_H