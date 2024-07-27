#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "global.c"
#include "server/server.h"
#include "utils/settings.h"
#include "utils/sstring.h"
#include "utils/utils.h"

int main() {
  global_max_req_size = 65535;
  global_max_res_size = 65535;

  printf("Starting server...\n");

  int settings_load = load_settings();
  if (settings_load != 0) {
    return 1;
  }

  printf("Settings loaded.\n");

  int result = -1;

  switch (global_server_type) {
    case 11:
      printf("Server type: %d (blocking, http)\n", global_server_type);
      printf("Sorry, this setup has not been implemented yet.\n");
      break;
    case 12:
      printf("Server type: %d (blocking, https)\n", global_server_type);
      printf("Sorry, this setup has not been implemented yet.\n");
      break;
    case 21:
      result = start_server_event_loop_http(global_setting_port);
      printf("Server type: %d (event loop, http)\n", global_server_type);
      break;
    case 22:
      printf("Server type: %d (event loop, https)\n", global_server_type);
      printf("Sorry, this setup has not been implemented yet.\n");
      break;
    default:
      printf("Server type: %d (unknown)\n", global_server_type);
      return 1;
  }

  if (result != 0) {
    fprintf(stderr, "Failed to start the server.\n");
    return 1;
  }

  free_settings();

  printf("Server closed. Port %d is free.\n\n", global_setting_port);

  return 0;
}