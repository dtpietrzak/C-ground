#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "server.h"

int main() {
  int result = start_server();
  if (result != 0) {
    fprintf(stderr, "Failed to start the server\n");
    return 1;
  }
  return 0;
}