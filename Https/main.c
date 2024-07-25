#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_CERT "server.crt"
#define SERVER_KEY "server.key"
#define SERVER_PORT 7443

void handle(SSL *ssl) {
  char res[] =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html; charset=UTF-8\r\n\r\n"
      "<!DOCTYPE html><html><head><title>Hello, World!</title></head>"
      "<body><h1>Hello, World!</h1></body></html>";

  SSL_write(ssl, res, strlen(res));
  SSL_shutdown(ssl);
  SSL_free(ssl);
}

int main(void) {
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

  SSL_CTX_use_certificate_file(ctx, SERVER_CERT, SSL_FILETYPE_PEM);
  SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY, SSL_FILETYPE_PEM);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERVER_PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Unable to bind");
    exit(EXIT_FAILURE);
    return 1;
  }

  listen(server_fd, 10);

  printf("Listening on port %d\n", SERVER_PORT);

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
