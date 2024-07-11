#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "global.c"
#include "server.h"
#include "utils/utils.h"

int main() {
  char* settings = read_file_to_string("settings.conf");
  int16_t port;

  if (settings == NULL) {
    settings = "port:7777\nauth:abc123\nip:127.0.0.1\n";
    save_string_to_file(settings, "settings.conf");
  }

  sscanf(settings, "port:%hu\nauth:%128s\nip:%s", &port, global_auth, global_ip);

  int result = start_server(port);

  free(settings);

  if (result != 0) {
    fprintf(stderr, "Failed to start the server\n");
    return 1;
  }
  return 0;
}