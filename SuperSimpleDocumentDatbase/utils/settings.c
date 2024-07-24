#include "settings.h"

#define DEFAULT_PORT 7777
#define DEFAULT_AUTH \
  "CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PATH "./"
#define SETTINGS_FILE_PATH "./settings.json"

char* settings_file_path = SETTINGS_FILE_PATH;

int load_string_setting(const JSON_Value* settings_json, char** global_setting,
                        const char* setting_name, int setting_max_length,
                        const char* settings_file_path) {
  int ip_length =
      json_object_get_string_len(json_object(settings_json), setting_name);
  if (ip_length > setting_max_length) {
    printf("Settings error: %s is too long, max length: %d (see: %s)\n",
           setting_name, setting_max_length, settings_file_path);
    return 1;
  }

  const char* ip_from_json =
      json_object_get_string(json_object(settings_json), setting_name);
  if (ip_from_json == NULL) {
    printf("Settings error: %s is null (see: %s)\n", setting_name,
           settings_file_path);
    return 1;
  }

  *global_setting = malloc(ip_length + 1);
  if (*global_setting == NULL) {
    printf("Settings error: failed to allocate memory for %s\n", setting_name);
    return 1;
  }

  strncpy(*global_setting, ip_from_json, ip_length);
  (*global_setting)[ip_length] = '\0';
  return 0;
}

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

      if (load_string_setting(settings_json, &global_setting_auth_ptr, "auth",
                              1024, settings_file_path) != 0) {
        return 1;
      }
      if (load_string_setting(settings_json, &global_setting_ip_ptr, "ip", 17,
                              settings_file_path) != 0) {
        return 1;
      }
      if (load_string_setting(settings_json, &global_setting_path_ptr, "path",
                              1024, settings_file_path) != 0) {
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
      global_setting_auth_ptr = (char*)malloc(auth_length);
      strncpy(global_setting_auth_ptr, DEFAULT_AUTH, auth_length);

      int ip_length = strlen(DEFAULT_IP);
      global_setting_ip_ptr = (char*)malloc(ip_length);
      strncpy(global_setting_ip_ptr, DEFAULT_IP, ip_length);

      int path_length = strlen(DEFAULT_PATH);
      global_setting_path_ptr = (char*)malloc(path_length);
      strncpy(global_setting_path_ptr, DEFAULT_PATH, path_length);

      char default_settings_str[1024];
      snprintf(default_settings_str, sizeof(default_settings_str),
               "{\n  \"port\": %d,\n  \"auth\": \"%s\",\n  \"ip\": \"%s\",\n  "
               "\"path\": \"%s\"\n}",
               DEFAULT_PORT, DEFAULT_AUTH, DEFAULT_IP, DEFAULT_PATH);
      save_string_to_file(default_settings_str, settings_file_path);

      printf("\ndefaults: \n");
      printf("  port: %hd\n", global_setting_port);
      printf("  auth: %s\n", global_setting_auth_ptr);
      printf("  ip: %s\n", global_setting_ip_ptr);
      printf("  path: %s\n\n", global_setting_path_ptr);

      return 0;
    } else {
      printf("Settings error - file access issue: %s\n", file_access_issue);
      return 1;
    }
  }

  return 0;
}

void free_settings() {
  if (global_setting_auth_ptr != NULL) {
    free(global_setting_auth_ptr);
  }
  if (global_setting_ip_ptr != NULL) {
    free(global_setting_ip_ptr);
  }
  if (global_setting_path_ptr != NULL) {
    free(global_setting_path_ptr);
  }
}