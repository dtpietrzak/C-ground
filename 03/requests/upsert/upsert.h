#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../http.h"
#include "../../utils/file_operations.h"
#include "../../utils/parson.h"
#include "../../utils/requests.h"
#include "../../utils/utils.h"
#include "utils.h"
#include "compile_dot_notation.h"
#include "compile_entire_doc.h"

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

int handle_request_upsert(HttpRequest* http_request,
                          HttpResponse* http_response);