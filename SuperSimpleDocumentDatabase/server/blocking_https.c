#include "blocking_https.h"

#include "../global.c"

void handle(SSL *ssl) {
  // get request
  char buffer[65536];
  int len = SSL_read(ssl, buffer, sizeof(buffer) - 1);
  if (len < 0) {
    perror("Unable to read");
    return;
  }
  debug_request_string(buffer);

  // start response_body
  SString response_str;
  s_init(&response_str, "", global_max_res_size);

  // Process the received data
  handle_request(buffer, &response_str);

  if (response_str.value != NULL) {
    debug_response_string(response_str.value);
    SSL_write(ssl, response_str.value, response_str.length);
  } else {
    printf("Failed to process request.\n");
  }

  SSL_shutdown(ssl);
  SSL_free(ssl);
}

int start_server_blocking_https(int port) {
  SSL_CTX *ctx;

  int server_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  SSL_library_init();
  SSL_load_error_strings();

  // start the magic
  ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
    return 1;
  }

  SSL_CTX_use_certificate_file(ctx, global_setting_path_for_crt_ptr,
                               SSL_FILETYPE_PEM);
  SSL_CTX_use_PrivateKey_file(ctx, global_setting_path_for_key_ptr,
                              SSL_FILETYPE_PEM);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Unable to bind");
    exit(EXIT_FAILURE);
    return 1;
  }

  listen(server_fd, 10);

  printf("Listening on port %d\n", port);

  while (1) {
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);

    if (SSL_accept(ssl) <= 0) {
      ERR_print_errors_fp(stderr);
    } else {
      handle(ssl);
    }
    close(client_fd);
  }

  close(server_fd);
  SSL_CTX_free(ctx);

  return 0;
}
