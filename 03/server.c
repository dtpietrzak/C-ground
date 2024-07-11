#include "server.h"

#include "global.c"

#define MAX_REQ_SIZE 1024
#define MAX_RES_SIZE 2048
#define AUTH_LENGTH 128

typedef char *(*RequestHandler)(HttpRequest *);

char *create_http_header(char *status_code, const char *content_type,
                         int content_length) {
  // Allocate memory for the response
  char *response = (char *)malloc(1024 * sizeof(char));
  if (response == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return NULL;
  }
  response[0] = '\0';  // Ensure the string is empty initially

  // Format HTTP status line
  sprintf(response, "HTTP/1.1 %s\r\n", status_code);

  // Append headers
  sprintf(response + strlen(response), "Content-Type: %s\r\n", content_type);
  sprintf(response + strlen(response), "Content-Length: %d\r\n",
          content_length);

  // End of headers
  strcat(response, "\r\n");

  return response;
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
  free(request->headers);
  free(request->body);
}

char *auth_error(const char *error_message) {
  char message[MAX_RES_SIZE];
  memset(message, 0, sizeof(message));
  sprintf(message, "{\"status\": \"%d\", \"reason\": \"%s\"}", 401,
          error_message);
  char *status_code = "401 Unauthorized";
  const char *content_type = "application/json";
  int content_length = strlen(message);

  char *http_response =
      create_http_header(status_code, content_type, content_length);
  if (http_response == NULL) {
    return NULL;
  }

  http_response =
      realloc(http_response, strlen(http_response) + strlen(message) + 1);
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

  if (strcmp(auth_header, global_auth)) {
    free(auth_header);
    return auth_error("Invalid authentication");
  }

  free(auth_header);

  return NULL;
}

const char *process_request(const char *request_str) {
  if (strlen(request_str) > MAX_REQ_SIZE) {
    printf("Request too large!");
    return NULL;
  }

  char *bad_auth_response = validate_auth_header(request_str);
  if (bad_auth_response != NULL) {
    return bad_auth_response;
  }

  HttpRequest request;

  parse_http_request(request_str, &request);

  char message[MAX_RES_SIZE];
  memset(message, 0, sizeof(message));
  char *response = handle_request(&request);

  char *status_code = "200 OK";
  if (!strcmp(response, "201")) {
    status_code = "201 Created";
    sprintf(message, "{\"status\": \"%s\"}", response);
  } else if (!strcmp(response, "204")) {
    status_code = "204 No Content";
    sprintf(message, "{\"status\": \"%s\"}", response);
  } else if (!strcmp(response, "404")) {
    status_code = "404 Not Found";
    sprintf(message, "{\"status\": \"%s\"}", response);
  } else if (!strcmp(response, "400")) {
    status_code = "400 Bad Request";
    sprintf(message, "{\"status\": \"%s\"}", response);
  } else {
    sprintf(message, "{\"status\": \"200\", \"body\": \"%s\"}", response);
  }

  const char *content_type = "application/json";
  int content_length = strlen(message);

  char *http_response =
      create_http_header(status_code, content_type, content_length);
  if (http_response == NULL) {
    return NULL;
  }

  http_response =
      realloc(http_response, strlen(http_response) + strlen(message) + 1);
  strcat(http_response, message);

  return http_response;
}

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
  if (buf->base) {
    memset(buf->base, 0, suggested_size);  // Zero out the buffer
  }
}

void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    printf("Request:\n%s\n\n", buf->base);

    // Process the received data
    const char *processed_data = process_request(buf->base);

    // Free buffer allocated by on_alloc_buffer
    if (buf->base) {
      free(buf->base);
    }

    if (processed_data != NULL) {
      printf("Response:\n%s\n\n\n", processed_data);

      uv_write_t write_req;
      uv_buf_t write_buf =
          uv_buf_init((char *)processed_data, strlen(processed_data));
      uv_write(&write_req, client_stream, &write_buf, 1, NULL);

      free((void *)processed_data);
    } else {
      printf("Failed to process request.\n");
      uv_close((uv_handle_t *)client_stream, NULL);
    }
  } else if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
    }
    uv_close((uv_handle_t *)client_stream, NULL);
  }
}

void on_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  if (!client) {
    fprintf(stderr, "Memory allocation error\n");
    return;
  }

  uv_tcp_init(uv_default_loop(), client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    struct sockaddr_storage peername;
    int namelen = sizeof(peername);

    if (uv_tcp_getpeername(client, (struct sockaddr *)&peername, &namelen) ==
        0) {
      char ip[17] = {'\0'};
      if (peername.ss_family == AF_INET) {
        uv_ip4_name((struct sockaddr_in *)&peername, ip, 16);
      } else if (peername.ss_family == AF_INET6) {
        uv_ip6_name((struct sockaddr_in6 *)&peername, ip, 16);
      }

      if (strcmp(global_ip, ip)) {
        printf("Client attempted illegal connection from IP: %s\n", ip);
        uv_close((uv_handle_t *)client, NULL);
        return;
      }
    } else {
      fprintf(stderr, "Failed to get peer name\n");
      uv_close((uv_handle_t *)client, NULL);
      return;
    }

    printf("\n\nAccepted connection!\n");
    uv_read_start((uv_stream_t *)client, on_alloc_buffer, on_read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

int start_server(int port) {
  uv_tcp_t server;
  uv_loop_t *loop = uv_default_loop();

  uv_tcp_init(loop, &server);

  struct sockaddr_in bind_addr;
  uv_ip4_addr("127.0.0.1", port, &bind_addr);
  uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);

  int r = uv_listen((uv_stream_t *)&server, SOMAXCONN, on_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }

  printf("Listening on port %d\n", port);
  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
