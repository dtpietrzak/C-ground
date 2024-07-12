#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "server.h"
#include "utils/settings.h"
#include "utils/utils.h"

int main() {
  load_settings();

  int result = start_server(global_setting_port);

  if (result != 0) {
    fprintf(stderr, "Failed to start the server\n");
    return 1;
  }
  return 0;
}