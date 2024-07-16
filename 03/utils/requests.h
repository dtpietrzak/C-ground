#ifndef UTILS_REQUESTS_H
#define UTILS_REQUESTS_H

#include <stdbool.h>
#include <string.h>

#include "../http.h"
#include "utils.h"

typedef struct {
  char *invalid;
  char *key;
  char *db;
} QueryParams;

QueryParams validate_queries(HttpRequest *http_request, char *requiredParams[],
                             int num_required);

#endif  // UTILS_REQUESTS_H