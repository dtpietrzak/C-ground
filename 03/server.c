#include "server.h"

#include "global.c"

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
    snprintf(message, MAX_RES_SIZE, "{\"status\": \"%s\"}", response);
  } else if (!strcmp(response, "204")) {
    status_code = "204 No Content";
    snprintf(message, MAX_RES_SIZE, "{\"status\": \"%s\"}", response);
  } else if (!strcmp(response, "404")) {
    status_code = "404 Not Found";
    snprintf(message, MAX_RES_SIZE, "{\"status\": \"%s\"}", response);
  } else if (!strcmp(response, "400")) {
    status_code = "400 Bad Request";
    snprintf(message, MAX_RES_SIZE, "{\"status\": \"%s\"}", response);
  } else {
    snprintf(message, MAX_RES_SIZE, "{\"status\": \"200\", \"body\": \"%s\"}",
             response);
  }

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

int validate_tcp_ip(uv_tcp_t *client) {
  struct sockaddr_storage peername;
  int namelen = sizeof(peername);

  if (uv_tcp_getpeername(client, (struct sockaddr *)&peername, &namelen) == 0) {
    char ip[17] = {'\0'};
    if (peername.ss_family == AF_INET) {
      uv_ip4_name((struct sockaddr_in *)&peername, ip, 16);
    } else if (peername.ss_family == AF_INET6) {
      uv_ip6_name((struct sockaddr_in6 *)&peername, ip, 16);
    }

    if (strcmp(global_setting_ip, ip) == 0) {
      return 0;
    } else {
      fprintf(stderr, "Client attempted illegal connection from IP: %s\n", ip);
      return 1;
    }
  } else {
    fprintf(stderr, "Failed to get peer name\n");
    return 1;
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

  if (uv_accept(server, (uv_stream_t *)client) == 0 &&
      validate_tcp_ip(client) == 0) {
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
