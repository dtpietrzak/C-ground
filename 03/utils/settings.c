#include "settings.h"

#define DEFAULT_PORT 7777
#define DEFAULT_AUTH                                                           \
  "W8G64bTO1Ae0KDyXFhZh9tJrW6Sui3B2p9G05C8htXtXF1DRBK0QzB0FoUfzI0818BMHbps4oO" \
  "3gIF2YNQXMJzwrDISXTpDqERAqE809n1gh5ggfTbKy76Hi2bjOWk55"
#define DEFAULT_IP "127.0.0.1"

// This sets global_settings based on settings.conf content or default settings
void load_settings() {
  char* settings_str = read_file_to_string("settings.conf");

  if (settings_str == NULL) {
    printf(
        "_database has automatically set default settings. Remember to check / "
        "update these before going to production! (see: settings.conf)");

    global_setting_port = DEFAULT_PORT;
    strncpy(global_setting_auth, DEFAULT_AUTH, strlen(DEFAULT_AUTH));
    strncpy(global_setting_ip, DEFAULT_IP, strlen(DEFAULT_IP));

    char default_settings_str[256];
    snprintf(default_settings_str, sizeof(default_settings_str),
             "port:%d\nauth:%s\nip:%s\n", DEFAULT_PORT, DEFAULT_AUTH,
             DEFAULT_IP);
    save_string_to_file(default_settings_str, "settings.conf");

    printf("port: %hu\n", global_setting_port);
    printf("auth: %s\n", global_setting_auth);
    printf("ip: %s\n", global_setting_ip);
  } else {
    sscanf(settings_str, "port:%hu\nauth:%128s\nip:%s", &global_setting_port,
           global_setting_auth, global_setting_ip);
  }

  free(settings_str);
}