#include "http.h"

void parse_http_request(const char *request_str, HttpRequest *request) {
  // Parse method and path
  sscanf(request_str, "%s %s", request->method, request->path);

  // Check if there are query parameters in the path
  char *query_start = strchr(request->path, '?');
  if (query_start != NULL) {
    *query_start = '\0';  // Separate path and query parameters
    query_start++;        // Move past the '?'

    // Parse query parameters
    char *param_pair = strtok(query_start, "&");
    int query_index = 0;
    while (param_pair != NULL && query_index < MAX_REQ_QUERIES) {
      char *equal_sign = strchr(param_pair, '=');
      if (equal_sign != NULL) {
        *equal_sign = '\0';
        equal_sign++;

        // id
        strncpy(request->queries[query_index][0], param_pair,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][0][MAX_REQ_QUERY_SIZE - 1] = '\0';

        // value
        strncpy(request->queries[query_index][1], equal_sign,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][1][MAX_REQ_QUERY_SIZE - 1] = '\0';
      } else {
        // No '=' found, treat as id with empty value
        strncpy(request->queries[query_index][0], param_pair,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][0][MAX_REQ_QUERY_SIZE - 1] = '\0';
        request->queries[query_index][1][0] = '\0';  // Empty value
      }
      query_index++;
      param_pair = strtok(NULL, "&");
    }
    request->num_queries = query_index;
  }

  // Parse headers
  const char *body_start = strstr(request_str, "\r\n\r\n");
  if (body_start != NULL) {
    // Copy headers
    const char *header_end = strstr(request_str, "\r\n");
    while (header_end != NULL && header_end < body_start) {
      header_end += 2;  // Move past "\r\n"
      const char *header_start = header_end;
      header_end = strstr(header_start, "\r\n");
      if (header_end != NULL) {
        int header_len = header_end - header_start;
        if (request->num_headers < MAX_REQ_HEADERS) {
          strncpy(request->headers[request->num_headers], header_start,
                  header_len);
          request->headers[request->num_headers][header_len] = '\0';
          request->num_headers++;
        }
      }
    }
    s_set(&request->body, body_start + 4);
  }
}

void compile_http_response(HttpResponse *http_response, SString *response_str) {
  // start status_code
  SString status_code;
  s_init(&status_code, "", 36);

  switch (http_response->status) {
    case 200:
      s_set(&status_code, "200 OK");
      break;
    case 201:
      s_set(&status_code, "201 Created");
      break;
    case 204:
      s_set(&status_code, "204 No Content");
      break;
    case 400:
      s_set(&status_code, "400 Bad Request");
      break;
    case 401:
      s_set(&status_code, "401 Unauthorized");
      break;
    case 404:
      s_set(&status_code, "404 Not Found");
      break;
    default:
      s_set(&status_code, "500 Internal Server Error");
      break;
  }

  SString response_content_str;
  s_init(&response_content_str, "", MAX_RES_SIZE);
  s_compile(&response_content_str, "{\"status\":\"%d\",\"body\":%s}",
            http_response->status, http_response->body.value);

  s_compile(response_str,
            "HTTP/1.1 %s\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            status_code.value, response_content_str.length,
            response_content_str.value);
}

void free_http_request(HttpRequest *http_request) {
  memset(&http_request->headers, 0, sizeof(http_request->headers));
  memset(&http_request->method, 0, sizeof(http_request->method));
  memset(&http_request->path, 0, sizeof(http_request->path));
  memset(&http_request->queries, 0, sizeof(http_request->queries));
  s_free(&http_request->body);
}

void free_http_response(HttpResponse *http_response) {
  http_response->status = 0;
  s_free(&http_response->body);
}

void validate_auth_header(const char *request_str,
                          HttpResponse *http_response) {
  const char *auth_start = strstr(request_str, "Authorization: ");
  if (auth_start == NULL) {
    http_response->status = 401;
    s_set(&http_response->body, "\"Authorization header not found\"");
    return;
  }

  auth_start += strlen("Authorization: ");
  const char *auth_end = strchr(auth_start, '\n');
  if (auth_end == NULL) {
    auth_end = strchr(auth_start, '\r');  // Handle different line endings
  }
  if (auth_end == NULL) {
    http_response->status = 401;
    s_set(&http_response->body, "\"Auth header end of line not found\"");
    return;
  }

  // Calculate length of auth header
  size_t auth_len = auth_end - auth_start;

  // Allocate memory for auth header + null terminator
  char *auth_header = (char *)malloc(auth_len + 1);
  if (auth_header == NULL) {
    free(auth_header);
    http_response->status = 401;
    s_set(&http_response->body, "\"Memory allocation failed\"");
    return;
  }

  // Copy auth header and null terminate
  strncpy(auth_header, auth_start, auth_len);
  auth_header[auth_len] = '\0';

  // Remove carriage return if present
  if (auth_header[auth_len - 1] == '\r') {
    auth_header[auth_len - 1] = '\0';
    auth_len--;
  }

  if (auth_header == NULL) {
    http_response->status = 401;
    s_set(&http_response->body,
          "\"Authorization header not found or invalid format\"");
  } else if (strcmp(auth_header, global_setting_auth)) {
    http_response->status = 401;
    s_set(&http_response->body, "\"Invalid authentication\"");
  } else if (auth_len < REQUIRED_AUTH_LENGTH) {
    http_response->status = 401;
    s_set(&http_response->body,
          "\"Invalid authentication header length, should be at least 32 "
          "characters\"");
  }

  free(auth_header);
}