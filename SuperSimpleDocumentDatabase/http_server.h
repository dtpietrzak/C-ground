#ifndef SERVER_H
#define SERVER_H

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "utils/http.h"
#include "utils/sstring.h"
#include "utils/utils.h"
#include "process_request.h"

#define MAX_RES_SIZE 65535
#define MAX_REQ_SIZE 65535

int start_server(int port);

#endif  // SERVER_H