#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sdb_utils.h>

#include "upsert_utils.h"
#include "compile_dot_notation.h"
#include "compile_entire_doc.h"

#ifdef _WIN32
#include <direct.h>  // For Windows mkdir
#define mkdir(directory, mode) _mkdir(directory)
#else
#include <libgen.h>    // For dirname
#include <sys/stat.h>  // For mkdir
#endif

int handle_request_upsert(sdb_http_request_t* http_request,
                          sdb_http_response_t* http_response);