#include "request_distributor.h"

#include "../delete/delete.h"
#include "../doc/doc.h"
#include "../find/find.h"
#include "../index/index_delete.h"
#include "../index/index_upsert.h"
#include "../query/_query_distributor/query_distributor.h"
#include "../schema/schema.h"
#include "../upsert/upsert.h"

// distributor handles the incoming request and routes it to the correct handler
// if there is a routing error it handles the status code and body
// if there is an error inside the handler it prepends the error message
// and wraps it in quotes

// so handlers are expected to return 0 on success and non-zero on failure
// if they return non-zero, the distributor will prepend the error message
// and wrap it in quotes

// 0 = move on (thank you, next)
// 1 = exit early
int endpoint(const char* path, const char* method,
             int (*handler)(HttpRequest*, HttpResponse*),
             const char* error_prefix, HttpRequest* http_request,
             HttpResponse* http_response) {
  if (strcmp(http_request->path, path) == 0) {
    if (strcmp(http_request->method, method) == 0) {
      if (handler(http_request, http_response) == 0) {
        return 1;
      } else {
        char* error_message = s_out(&http_response->body);
        s_compile(&http_response->body, "\"%s%s\"", error_prefix,
                  error_message);
        free(error_message);
        return 1;
      }
    } else {
      http_response->status = 400;
      s_compile(&http_response->body,
                "\"Invalid method. Requests to %s should be %s\"", path,
                method);
      return 1;
    }
  }
  return 0;
}

void distribute_request(HttpRequest* http_request,
                        HttpResponse* http_response) {
  if (endpoint("/query", "GET", handle_request_query,
               "Failed to query documents: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/find", "GET", handle_request_find,
               "Failed to find document: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/doc", "GET", handle_request_doc,
               "Failed to get document: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/upsert", "POST", handle_request_upsert,
               "Failed to upsert document: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/delete", "DELETE", handle_request_delete,
               "Failed to delete document: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/schema", "POST", handle_request_schema,
               "Failed to save schema: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/index/upsert", "POST", handle_request_index_upsert,
               "Failed to save index: ", http_request, http_response)) {
    return;
  }

  if (endpoint("/index/delete", "DELETE", handle_request_index_delete,
               "Failed to delete index: ", http_request, http_response)) {
    return;
  }

  http_response->status = 404;
  s_set(&http_response->body, "\"Endpoint not found\"");
  return;
}

void handle_request(const char* request_str, SString* response_str) {
  // start response
  HttpResponse http_response;
  s_init(&http_response.body, "", MAX_RES_SIZE);

  if (request_str == NULL) {
    http_response.status = 400;
    s_set(&http_response.body, "Empty request");
  } else if (strlen(request_str) > MAX_REQ_SIZE) {
    http_response.status = 400;
    s_set(&http_response.body, "Request too large");
  } else {
    validate_auth_header(request_str, &http_response);
    if (http_response.status != 401) {
      // start request
      HttpRequest http_request;
      // Initialize request struct
      memset(&http_request, 0, sizeof(HttpRequest));
      s_init(&http_request.body, "", MAX_REQ_BODY_SIZE);
      // parse request string into request struct
      parse_http_request(request_str, &http_request);
      // handle the request
      distribute_request(&http_request, &http_response);
      free_http_request(&http_request);
      // end request
    }
  }

  // compile http_request into the request string
  compile_http_response(&http_response, response_str);

  free_http_response(&http_response);
  // end response
}