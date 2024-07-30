#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../global.c"
#include "../request_state/request_state.h"
#include "../string/contains.h"
#include "../string/sstring.h"

// handles http string parsing and validation

void validate_auth_header(const char *request_str,
                          sdb_http_response_t *http_response);
sdb_query_params_t validate_and_parse_queries(sdb_http_request_t *http_request,
                                              const char *params[],
                                              int num_of_params);
void parse_http_request(const char *request_str, sdb_http_request_t *request);

void compile_http_response(sdb_http_response_t *http_response,
                           SString *response_str);

char *url_encode(const char *str);
char *url_decode(const char *str);

#endif  // HTTP_HTTP_H