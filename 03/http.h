#ifndef HTTP_H
#define HTTP_H

#define MAX_REQ_METHOD_SIZE 16
#define MAX_REQ_PATH_SIZE 256
#define MAX_REQ_QUERIES 24
#define MAX_REQ_QUERY_SIZE 256
#define MAX_REQ_HEADERS 12
#define MAX_REQ_HEADER_SIZE 256
#define MAX_REQ_BODY_SIZE 1024

typedef struct {
  char method[MAX_REQ_METHOD_SIZE];
  char path[MAX_REQ_PATH_SIZE];
  char queries[MAX_REQ_QUERIES][2][MAX_REQ_QUERY_SIZE];
  int num_queries;
  char headers[MAX_REQ_HEADERS][MAX_REQ_HEADER_SIZE];
  int num_headers;
  char body[MAX_REQ_BODY_SIZE];
} HttpRequest;

#endif  // HTTP_H