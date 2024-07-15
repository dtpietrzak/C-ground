#include "settings.h"

#define DEFAULT_PORT 7777
#define DEFAULT_AUTH \
  "CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PATH "./"
#define SETTINGS_FILE_PATH "./settings.json"

char* settings_file_path = SETTINGS_FILE_PATH;

// This sets global_settings based on settings.conf content or default
// settings
int load_settings() {
  char* file_access_issue = check_file_access(settings_file_path, 1);
  if (file_access_issue == NULL) {
    char* settings_str = read_file_to_string(settings_file_path);

    if (settings_str == NULL) {
      printf("Settings error - file read issue: %s\n", settings_file_path);
    } else {
      JSON_Value* settings_json = json_parse_string(settings_str);
      if (settings_json == NULL) {
        return 1;
      }

      global_setting_port =
          json_object_get_number(json_object(settings_json), "port");
      if (global_setting_port == 0) {
        printf(
            "Settings error: failed on \"port\", should be a valid port "
            "integer\n");
        return 1;
      }

      int auth_length =
          json_object_get_string_len(json_object(settings_json), "auth");
      global_setting_auth = malloc(auth_length);
      strncpy(global_setting_auth,
              json_object_get_string(json_object(settings_json), "auth"),
              auth_length);
      if (global_setting_auth == NULL) {
        printf("Settings error: failed on \"auth\"\n");
        return 1;
      }

      int ip_length =
          json_object_get_string_len(json_object(settings_json), "ip");
      global_setting_ip = malloc(ip_length);
      strncpy(global_setting_ip,
              json_object_get_string(json_object(settings_json), "ip"),
              ip_length);
      if (global_setting_ip == NULL) {
        printf("Settings error: failed on \"auth\"\n");
        return 1;
      }

      int path_length =
          json_object_get_string_len(json_object(settings_json), "path");
      global_setting_path = malloc(path_length);
      strncpy(global_setting_path,
              json_object_get_string(json_object(settings_json), "path"),
              path_length);
      if (global_setting_path == NULL) {
        printf("Settings error: failed on \"auth\"\n");
        return 1;
      }
      size_t path_len = strlen(global_setting_path);
      if (path_len > 0 && global_setting_path[path_len - 1] != '/') {
        printf("Settings error: your path does not end in \"/\" (see: %s)\n",
               settings_file_path);
        return 1;
      }

      json_value_free(settings_json);
    }

    free(settings_str);
  } else {
    if (strcmp(file_access_issue, "Document does not exist") == 0) {
      printf(
          "\n_database has automatically set default settings. Remember to "
          "check / update these before going to production! (see: %s)\n",
          settings_file_path);

      global_setting_port = DEFAULT_PORT;

      int auth_length = strlen(DEFAULT_AUTH);
      global_setting_auth = malloc(auth_length);
      strncpy(global_setting_auth, DEFAULT_AUTH, auth_length);

      int ip_length = strlen(DEFAULT_IP);
      global_setting_ip = malloc(ip_length);
      strncpy(global_setting_ip, DEFAULT_IP, ip_length);

      int path_length = strlen(DEFAULT_PATH);
      global_setting_path = malloc(path_length);
      strncpy(global_setting_path, DEFAULT_PATH, path_length);

      char default_settings_str[1024];
      snprintf(default_settings_str, sizeof(default_settings_str),
               "{\n  \"port\": %d,\n  \"auth\": \"%s\",\n  \"ip\": \"%s\",\n  "
               "\"path\": \"%s\"\n}",
               DEFAULT_PORT, DEFAULT_AUTH, DEFAULT_IP, DEFAULT_PATH);
      save_string_to_file(default_settings_str, settings_file_path);

      printf("\ndefaults: \n");
      printf("  port: %hu\n", global_setting_port);
      printf("  auth: %s\n", global_setting_auth);
      printf("  ip: %s\n", global_setting_ip);
      printf("  path: %s\n\n", global_setting_path);

      return 0;
    } else {
      printf("Settings error - file access issue: %s\n", file_access_issue);
      return 1;
    }
  }

  return 0;
}

void free_settings() {
  if (global_setting_ip != NULL) {
    free(global_setting_ip);
  }
  if (global_setting_path != NULL) {
    free(global_setting_path);
  }
}