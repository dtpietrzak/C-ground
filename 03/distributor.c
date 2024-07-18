#include "distributor.h"

#include "requests/delete/delete.h"
#include "requests/doc/doc.h"
#include "requests/index/index.h"
#include "requests/schema/schema.h"
#include "requests/upsert/upsert.h"

// distributor handles the incoming request and routes it to the correct handler
// if there is a routing error it handles the status code and body
// if there is an error inside the handler it prepends the error message
// and wraps it in quotes

// so handlers are expected to return 0 on success and non-zero on failure
// if they return non-zero, the distributor will prepend the error message
// and wrap it in quotes

// 0 = move on (thank you, next)
// 1 = exit early
int endpoint(HttpRequest* http_request, HttpResponse* http_response,
             const char* path, const char* method,
             int (*handler)(HttpRequest*, HttpResponse*),
             const char* error_prefix) {
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

void handle_request(HttpRequest* http_request, HttpResponse* http_response) {
  if (endpoint(http_request, http_response, "/doc", "GET", handle_request_doc,
               "Failed to get document: ")) {
    return;
  }

  if (endpoint(http_request, http_response, "/upsert", "POST",
               handle_request_upsert, "Failed to upsert document: ")) {
    return;
  }

  if (endpoint(http_request, http_response, "/delete", "DELETE",
               handle_request_delete, "Failed to delete document: ")) {
    return;
  }

  if (endpoint(http_request, http_response, "/schema", "POST",
               handle_request_schema, "Failed to save schema: ")) {
    return;
  }

  if (endpoint(http_request, http_response, "/index", "POST",
               handle_request_index, "Failed to index: ")) {
    return;
  }

  http_response->status = 404;
  s_set(&http_response->body, "\"Endpoint not found\"");
  return;
}
