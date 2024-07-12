#ifndef UTILS_REQUESTS_H
#define UTILS_REQUESTS_H

#include <stdbool.h>

#include "../http.h"
#include "utils.h"

typedef struct {
  bool valid;
  char *key;
  char *db;
} QueryParams;

QueryParams validate_queries(HttpRequest *http_request);

#endif  // UTILS_REQUESTS_H