#include <stdio.h>
#include <string.h>

#include "utils/http.h"

// http_server.h / https_server.h handle the server logic HTTP / HTTPS
// process_request.h handles the request logic
// it receives a request string and returns a response string
void process_request(const char* request_str, SString* response_str);