#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.c"

#define MAX_REQ_METHOD_SIZE 16
#define MAX_REQ_PATH_SIZE 256
#define MAX_REQ_QUERIES 24
#define MAX_REQ_QUERY_SIZE 256
#define MAX_REQ_HEADERS 12
#define MAX_REQ_HEADER_SIZE 256
#define MAX_REQ_BODY_SIZE 1024

#define MAX_RES_SIZE 65535
#define MAX_REQ_SIZE 65535

#define AUTH_LENGTH 128

typedef struct {
  char method[MAX_REQ_METHOD_SIZE];
  char path[MAX_REQ_PATH_SIZE];
  char queries[MAX_REQ_QUERIES][2][MAX_REQ_QUERY_SIZE];
  int num_queries;
  char headers[MAX_REQ_HEADERS][MAX_REQ_HEADER_SIZE];
  int num_headers;
  char body[MAX_REQ_BODY_SIZE];
} HttpRequest;

char *create_http_header(char *status_code, const char *content_type,
                         int content_length);
void parse_http_request(const char *request_str, HttpRequest *request);
void free_http_request(HttpRequest *request);
char *auth_error(const char *error_message);
char *validate_auth_header(const char *http_request);

#endif  // HTTP_H