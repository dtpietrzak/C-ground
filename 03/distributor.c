#include "distributor.h"

#include "requests/delete.h"
#include "requests/doc.h"
#include "requests/index.h"
#include "requests/schema.h"
#include "requests/upsert/upsert.h"

// distributor handles the incoming request and routes it to the correct handler
// if there is a routing error it handles the status code and body
// if there is an error inside the handler it prepends the error message
// and wraps it in quotes

// so handlers are expected to return 0 on success and non-zero on failure
// if they return non-zero, the distributor will prepend the error message
// and wrap it in quotes

void handle_request(HttpRequest* http_request, HttpResponse* http_response) {
  if (strcmp(http_request->path, "/doc") == 0) {
    if (strcmp(http_request->method, "GET") == 0) {
      if (handle_request_doc(http_request, http_response) != 0) {
        s_before_and_after(&http_response->body, "\"Failed to get doc: ", "\"");
      }
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /doc should be GET\"");
    }
  } else if (strcmp(http_request->path, "/upsert") == 0) {
    if (strcmp(http_request->method, "POST") == 0) {
      if (handle_request_upsert(http_request, http_response) != 0) {
        s_before_and_after(&http_response->body, "\"Failed to upsert: ", "\"");
      }
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /upsert should be POST\"");
    }
  } else if (strcmp(http_request->path, "/delete") == 0) {
    if (strcmp(http_request->method, "DELETE") == 0) {
      if (handle_request_delete(http_request, http_response) != 0) {
        s_before_and_after(&http_response->body, "\"Failed to delete: ", "\"");
      }
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /delete should be DELETE\"");
    }
  } else if (strcmp(http_request->path, "/schema") == 0) {
    if (strcmp(http_request->method, "POST") == 0) {
      if (handle_request_schema(http_request, http_response) != 0) {
        s_before_and_after(&http_response->body,
                           "\"Failed to save schema: ", "\"");
      }
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /delete should be POST\"");
    }
  } else if (strcmp(http_request->path, "/index") == 0) {
    if (strcmp(http_request->method, "POST") == 0) {
      if (handle_request_index(http_request, http_response) != 0) {
        s_before_and_after(&http_response->body, "\"Failed to index: ", "\"");
      }
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /index should be POST\"");
    }
  } else {
    http_response->status = 404;
    s_set(&http_response->body, "\"Endpoint not found\"");
  }
  // heads up, we still get here because we don't return earlier
  // each if statement else's through
  // so it should ever only end down one branch
}