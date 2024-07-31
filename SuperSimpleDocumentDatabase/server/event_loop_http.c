#include "event_loop_http.h"

#include "../global.c"

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  buf->base = (char *)mem_malloc(suggested_size, "on_alloc_buffer", 2);
  buf->len = suggested_size;
  if (buf->base) {
    memset(buf->base, 0, suggested_size);  // Zero out the buffer
  }
}

void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    // looks like I need a new allocation here to handle multiple chunks
    // seems weird, why wouldnt libuv handle this? shrug. more research needed

    debug_request_string(buf->base);

    // start response_body
    SString response_str;
    s_init(&response_str, "", global_max_res_size);

    // Process the received data
    handle_request(buf->base, &response_str);

    if (response_str.value != NULL) {
      debug_response_string(response_str.value);

      uv_write_t write_req;
      uv_buf_t write_buf = uv_buf_init(response_str.value, response_str.length);
      int write_status =
          uv_write(&write_req, client_stream, &write_buf, 1, NULL);
      if (write_status != 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror(write_status));
      }
      s_free(&response_str);
    } else {
      printf("Failed to process request.\n");
    }
  } else if (nread == UV_EOF) {
    uv_close((uv_handle_t *)client_stream, NULL);
    mem_free(client_stream, "client_tcp_buffer", 2);
  } else if (nread < 0) {  // error case
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
    }
  }

  if (buf->base) {
    mem_free(buf->base, "on_alloc_buffer", 2);
  }
}

int validate_tcp_ip(const uv_tcp_t *client) {
  struct sockaddr_storage peername;
  int namelen = sizeof(peername);

  if (uv_tcp_getpeername(client, (struct sockaddr *)&peername, &namelen) == 0) {
    char ip[17] = {'\0'};
    if (peername.ss_family == AF_INET) {
      uv_ip4_name((struct sockaddr_in *)&peername, ip, 16);
    } else if (peername.ss_family == AF_INET6) {
      uv_ip6_name((struct sockaddr_in6 *)&peername, ip, 16);
    }

    if (strncmp(global_setting_ip_ptr, ip, 18) == 0) {
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

void on_connection(uv_stream_t *server_stream, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  uv_tcp_t *client_stream =
      (uv_tcp_t *)mem_malloc(sizeof(uv_tcp_t), "client_tcp_buffer", 2);
  if (!client_stream) {
    fprintf(stderr, "Memory allocation error\n");
    return;
  }

  uv_tcp_init(uv_default_loop(), client_stream);

  if (uv_accept(server_stream, (uv_stream_t *)client_stream) == 0 &&
      validate_tcp_ip(client_stream) == 0) {
    uv_read_start((uv_stream_t *)client_stream, on_alloc_buffer, on_read);
  } else {
    uv_close((uv_handle_t *)client_stream, NULL);
    mem_free(client_stream, "client_tcp_buffer", 2);
  }
}

uv_tcp_t server;
uv_signal_t sigint;

volatile sig_atomic_t stop_server = 0;

// Signal handler function
void handle_sigint(uv_signal_t *handle, int signum) {
  uv_loop_t *loop = uv_default_loop();
  stop_server = 1;
  if (signum == SIGINT) {
    printf("\nClosing the server...\n");
  } else {
    printf("\nSIGINT received: %d\n", signum);
  }
  uv_signal_stop(handle);             // Stop receiving further SIGINT signals
  uv_tcp_close_reset(&server, NULL);  // Close the server
  uv_stop(loop);                      // Stop the event loop
}

int start_server_event_loop_http(int port) {
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

  uv_signal_init(loop, &sigint);
  uv_signal_start(&sigint, handle_sigint, SIGINT);

  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
