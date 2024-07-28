#include <stdio.h>
#include <string.h>

#include "../../utils/http.h"

// http_server.h / https_server.h handle the server logic HTTP / HTTPS
// request_distributor.h handles the request logic
// it receives a request string and returns a response string
void handle_request(const char* request_str, SString* response_str);