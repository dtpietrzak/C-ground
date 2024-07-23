#ifndef HTTP_H
#define HTTP_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.c"
#include "utils/sstring.h"
#include "utils/utils.h"

#define MAX_REQ_METHOD_SIZE 8
#define MAX_REQ_PATH_SIZE 1024
#define MAX_REQ_QUERIES 24
#define MAX_REQ_QUERY_SIZE 512
#define MAX_REQ_HEADERS 24
#define MAX_REQ_HEADER_SIZE 512
#define MAX_REQ_BODY_SIZE 39927

#define MAX_RES_SIZE 65535
#define MAX_REQ_SIZE 65535

#define REQUIRED_AUTH_LENGTH 32

typedef struct {
  char method[MAX_REQ_METHOD_SIZE];
  char path[MAX_REQ_PATH_SIZE];
  char queries[MAX_REQ_QUERIES][2][MAX_REQ_QUERY_SIZE];
  int num_queries;
  char headers[MAX_REQ_HEADERS][MAX_REQ_HEADER_SIZE];
  int num_headers;
  SString body;
} HttpRequest;

typedef struct {
  uint16_t status;
  SString body;
} HttpResponse;

void validate_auth_header(const char *request_str, HttpResponse *http_response);
void parse_http_request(const char *request_str, HttpRequest *request);
void compile_http_response(HttpResponse *http_response, SString *response_str);
void free_http_request(HttpRequest *request);
void free_http_response(HttpResponse *http_response);
char *url_encode(const char *str);
char *url_decode(const char *str);

#endif  // HTTP_H