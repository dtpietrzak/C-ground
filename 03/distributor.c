#include "distributor.h"

#include "requests/delete.h"
#include "requests/query.h"
#include "requests/upsert.h"

void handle_request(HttpRequest* http_request, HttpResponse* http_response) {
  if (!strcmp(http_request->path, "/query")) {
    if (!strcmp(http_request->method, "GET")) {
      handle_request_query(http_request, http_response);
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /query should be GET\"");
    }
  } else if (!strcmp(http_request->path, "/upsert")) {
    if (!strcmp(http_request->method, "POST")) {
      handle_request_insert(http_request, http_response);
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /upsert should be POST\"");
    }
  } else if (!strcmp(http_request->path, "/delete")) {
    if (!strcmp(http_request->method, "DELETE")) {
      handle_request_delete(http_request, http_response);
    } else {
      http_response->status = 400;
      s_set(&http_response->body,
            "\"Invalid method. Requests to /delete should be DELETE\"");
    }
  }
  // heads up, we still get here because we don't return earlier
  // each if statement else's through
  // so it should ever only end down one branch
}