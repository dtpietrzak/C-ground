#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../http.h"
#include "../utils/file_operations.h"
#include "../utils/requests.h"
#include "../utils/utils.h"

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

void handle_request_insert(HttpRequest* http_request,
                           HttpResponse* http_response);