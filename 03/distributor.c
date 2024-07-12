#include "distributor.h"

#include "requests/query.h"
#include "requests/upsert.h"
#include "requests/delete.h"

char* handle_request(HttpRequest* http_request) {
  if (!strcmp(http_request->path, "/query")) {
    if (!strcmp(http_request->method, "GET")) {
      return handle_request_query(http_request);
    } else {
      return "400";
    }
  } else if (!strcmp(http_request->path, "/upsert")) {
    if (!strcmp(http_request->method, "POST")) {
      return handle_request_insert(http_request);
    } else {
      return "400";
    }
  } else if (!strcmp(http_request->path, "/delete")) {
    if (!strcmp(http_request->method, "DELETE")) {
      return handle_request_delete(http_request);
    } else {
      return "400";
    }
  }
  return "404";
}