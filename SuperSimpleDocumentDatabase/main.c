#include <sdb_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "global.c"
#include "server/_server_distributor/server_distributor.h"
#include "settings/settings.h"

int main() {
  mem_check_init();
  global_max_req_size = 65535;
  global_max_res_size = 65535;

  printf("Starting server...\n");

  int settings_load = load_settings();
  if (settings_load != 0) {
    return 1;
  }

  int server_result = run_server();
  if (server_result != 0) {
    fprintf(stderr, "Failed to start the server.\n");
    return 1;
  }

  free_settings();

  printf("Server closed. Port %d is free.\n\n", global_setting_port);

  return 0;
}