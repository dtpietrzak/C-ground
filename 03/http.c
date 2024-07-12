#include "http.h"

char *create_http_header(char *status_code, const char *content_type,
                         int content_length) {
  char *http_header = (char *)malloc(MAX_RES_SIZE);
  if (http_header == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return NULL;
  }
  http_header[0] = '\0';  // Ensure the string is empty initially

  // Format HTTP status line
  snprintf(http_header, MAX_RES_SIZE, "HTTP/1.1 %s\r\n", status_code);

  // Append headers
  snprintf(http_header + strlen(http_header), MAX_RES_SIZE,
           "Content-Type: %s\r\n", content_type);
  snprintf(http_header + strlen(http_header), MAX_RES_SIZE,
           "Content-Length: %d\r\n", content_length);

  // End of headers
  strcat(http_header, "\r\n");

  return http_header;
}

void parse_http_request(const char *request_str, HttpRequest *request) {
  // Initialize request struct
  memset(request, 0, sizeof(HttpRequest));
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

        // key
        strncpy(request->queries[query_index][0], param_pair,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][0][MAX_REQ_QUERY_SIZE - 1] = '\0';

        // value
        strncpy(request->queries[query_index][1], equal_sign,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][1][MAX_REQ_QUERY_SIZE - 1] = '\0';
      } else {
        // No '=' found, treat as key with empty value
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

    // Copy body
    size_t body_length = strlen(body_start + 4);
    if (body_length < sizeof(request->body)) {
      strcpy(request->body, body_start + 4);  // Move past "\r\n\r\n"
    } else {
      strncpy(request->body, body_start + 4, sizeof(request->body) - 1);
      request->body[sizeof(request->body) - 1] =
          '\0';  // Ensure null termination
    }
  }
}

void free_http_request(HttpRequest *request) {
  free(request->method);
  free(request->path);
  free(request->queries);
  free(request->headers);
  free(request->body);
}

char *auth_error(const char *error_message) {
  char message[MAX_RES_SIZE];
  memset(message, 0, sizeof(message));
  snprintf(message, MAX_RES_SIZE, "{\"status\": \"%d\", \"reason\": \"%s\"}",
           401, error_message);
  char *status_code = "401 Unauthorized";
  const char *content_type = "application/json";
  int content_length = strlen(message);

  char *http_header =
      create_http_header(status_code, content_type, content_length);
  if (http_header == NULL) {
    free(http_header);
    return NULL;
  }

  char *http_response =
      (char *)malloc(strlen(http_header) + strlen(message) + 1);
  if (http_response == NULL) {
    free(http_header);
    return NULL;
  }

  strcat(http_response, http_header);
  strcat(http_response, message);

  return http_response;
}

char *validate_auth_header(const char *http_request) {
  const char *auth_start = strstr(http_request, "Authorization: ");
  if (auth_start == NULL) {
    return auth_error("Authorization header not found");
  }

  auth_start += strlen("Authorization: ");
  const char *auth_end = strchr(auth_start, '\n');
  if (auth_end == NULL) {
    auth_end = strchr(auth_start, '\r');  // Handle different line endings
  }
  if (auth_end == NULL) {
    return auth_error("End of line not found");
  }

  // Calculate length of auth header
  size_t auth_len = auth_end - auth_start;

  // Allocate memory for auth header + null terminator
  char *auth_header = (char *)malloc(auth_len + 1);
  if (auth_header == NULL) {
    free(auth_header);
    return auth_error("Memory allocation failed");
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
    free(auth_header);
    return auth_error("Authorization header not found or invalid format");
  }

  if (auth_len != AUTH_LENGTH) {
    free(auth_header);
    return auth_error("Invalid authentication header length");
  }

  if (strcmp(auth_header, global_setting_auth)) {
    free(auth_header);
    return auth_error("Invalid authentication");
  }

  free(auth_header);

  return NULL;
}