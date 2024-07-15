#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "server.h"
#include "utils/settings.h"
#include "utils/sstring.h"
#include "utils/utils.h"

int main() {
  int settings_load = load_settings();
  if (settings_load != 0) {
    return 1;
  }

  int result = start_server(global_setting_port);

  if (result != 0) {
    fprintf(stderr, "Failed to start the server.\n");
    return 1;
  }

  free_settings();

  return 0;
}