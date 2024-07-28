#include "server_distributor.h"

#include "../../global.c"

int run_server(void) {
  int server_type = 0;  // 11, 12, 21, 22

  if (strcmp(global_setting_server_algorithm_ptr, "blocking") == 0) {
    if (strcmp(global_setting_server_protocol_ptr, "http") == 0) {
      server_type = 11;
    } else if (strcmp(global_setting_server_protocol_ptr, "https") == 0) {
      server_type = 12;
    }
  } else if (strcmp(global_setting_server_algorithm_ptr, "event loop") == 0) {
    if (strcmp(global_setting_server_protocol_ptr, "http") == 0) {
      server_type = 21;
    } else if (strcmp(global_setting_server_protocol_ptr, "https") == 0) {
      server_type = 22;
    }
  }

  switch (server_type) {
    case 11:
      printf("Server type: %d (blocking, http)\n", server_type);
      return start_server_blocking_http(global_setting_port);
    case 12:
      printf("Server type: %d (blocking, https)\n", server_type);
      return start_server_blocking_https(global_setting_port);
    case 21:
      printf("Server type: %d (event loop, http)\n", server_type);
      return start_server_event_loop_http(global_setting_port);
    case 22:
      printf("Server type: %d (event loop, https)\n", server_type);
      printf("Sorry, this setup has not been implemented yet.\n");
      return 1;
    default:
      printf("Server type: %d (unknown)\n", server_type);
      return 1;
  }
}