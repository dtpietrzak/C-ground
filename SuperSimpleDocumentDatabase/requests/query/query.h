#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../utils/http.h"
#include "../../utils/file_operations.h"
#include "../../utils/parson.h"
#include "../../utils/request_operations.h"
#include "../../utils/utils.h"

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

int handle_request_query(HttpRequest* http_request,
                         HttpResponse* http_response);