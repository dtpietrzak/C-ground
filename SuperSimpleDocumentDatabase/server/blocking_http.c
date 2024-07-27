#include "blocking_http.h"

int server_fd;

void handle_signal(int signal) {
  if (signal == SIGINT) {
    printf("\nShutting down the server...\n");
    close(server_fd);
    exit(EXIT_SUCCESS);
  }
}

int start_server_blocking_http(int port) {
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("Unable to create socket");
    return 1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Unable to bind");
    return 1;
  }

  if (listen(server_fd, 10) < 0) {
    perror("Unable to listen");
    close(server_fd);
    return 1;
  }

  printf("Listening on port %d\n", port);

  // Register signal handler for graceful shutdown
  signal(SIGINT, handle_signal);

  while (1) {
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

    if (client_fd < 0) {
      perror("Unable to accept");
      return 1;
    }

    char buffer[65536];
    int len = read(client_fd, buffer, sizeof(buffer) - 1);
    if (len < 0) {
      perror("Unable to read");
      return 1;
    }

    buffer[len] = '\0';
    printf("Received: %s\n", buffer);

    // Send a basic HTTP response
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";
    write(client_fd, response, strlen(response));

    close(client_fd);
  }

  close(server_fd);
  return 0;
}
