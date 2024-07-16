#include "server.h"

#include "global.c"

void process_request(const char *request_str, SString *response_str) {
  // start response
  HttpResponse http_response;
  s_init(&http_response.body, "", MAX_RES_SIZE);

  if (strlen(request_str) > MAX_REQ_SIZE) {
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
      handle_request(&http_request, &http_response);
      free_http_request(&http_request);
      // end request
    }
  }

  // compile http_request into the request string
  compile_http_response(&http_response, response_str);

  free_http_response(&http_response);
  // end response
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

    // start response_body
    SString response_str;
    s_init(&response_str, "", MAX_RES_SIZE);

    // Process the received data
    process_request(buf->base, &response_str);

    // Free buffer allocated by on_alloc_buffer
    if (buf->base) {
      free(buf->base);
    }

    if (response_str.value != NULL) {
      printf("Response:\n%s\n\n\n", response_str.value);

      uv_write_t write_req;
      uv_buf_t write_buf = uv_buf_init(response_str.value, response_str.length);
      uv_write(&write_req, client_stream, &write_buf, 1, NULL);

      s_free(&response_str);
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
    fprintf(stderr, "\nListen error %s\n", uv_strerror(r));
    return 1;
  }

  printf("\nListening on port %d\n", global_setting_port);
  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
