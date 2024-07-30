#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sdb_utils.h>

#include "../query_number.h"

int handle_request_query(sdb_http_request_t* http_request,
                         sdb_http_response_t* http_response);