#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "../process_request.h"
#include "../utils/http.h"
#include "../utils/sstring.h"
#include "../utils/utils.h"

int start_server_event_loop_http(int port);