#include "settings.h"

#define DEFAULT_PORT 7777
#define DEFAULT_AUTH \
  "CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS_CHANGE_THIS"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PATH_DATA "./"
#define DEFAULT_PATH_CRT "./server.crt"
#define DEFAULT_PATH_KEY "./server.key"
#define DEFAULT_SERVER_ALGORITHM "blocking"
#define DEFAULT_SERVER_PROTOCOL "http"

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

int load_settings_from_file() {
  char* settings_str = read_file_to_string(settings_file_path);

  printf("Loading settings from: %s\n", settings_file_path);

  if (settings_str == NULL) {
    printf("Settings error - file read issue: %s\n", settings_file_path);
  } else {
    JSON_Value* settings_json = json_parse_string_with_comments(settings_str);
    if (settings_json == NULL) {
      printf("Settings error: failed to parse JSON (see: %s)\n",
             settings_file_path);
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
      printf("Settings error: failed on \"auth\"\n");
      return 1;
    }
    if (load_string_setting(settings_json, &global_setting_ip_ptr, "ip", 17,
                            settings_file_path) != 0) {
      printf("Settings error: failed on \"ip\"\n");
      return 1;
    }
    if (load_string_setting(settings_json, &global_setting_path_for_data_ptr,
                            "path_for_data", 1024, settings_file_path) != 0) {
      printf("Settings error: failed on \"path_for_data\"\n");
      return 1;
    }
    if (load_string_setting(settings_json, &global_setting_path_for_crt_ptr,
                            "path_for_crt", 1024, settings_file_path) != 0) {
      printf("Settings error: failed on \"path_for_crt\"\n");
      return 1;
    }
    if (load_string_setting(settings_json, &global_setting_path_for_key_ptr,
                            "path_for_key", 1024, settings_file_path) != 0) {
      printf("Settings error: failed on \"path_for_key\"\n");
      return 1;
    }
    if (load_string_setting(settings_json, &global_setting_server_algorithm_ptr,
                            "server_algorithm", 1024,
                            settings_file_path) != 0) {
      printf("Settings error: failed on \"server_algorithm\"\n");
      return 1;
    }
    if (load_string_setting(settings_json, &global_setting_server_protocol_ptr,
                            "server_protocol", 1024, settings_file_path) != 0) {
      printf("Settings error: failed on \"server_protocol\"\n");
      return 1;
    }

    json_value_free(settings_json);
  }

  free(settings_str);
  return 0;
}

int intialize_default_settings(const char* file_access_issue) {
  if (strcmp(file_access_issue, "Document does not exist") == 0) {
    printf("Settings file not found: %s\n", settings_file_path);
    printf("Creating default settings file: %s\n", settings_file_path);

    global_setting_port = DEFAULT_PORT;

    int auth_length = strlen(DEFAULT_AUTH);
    global_setting_auth_ptr = (char*)malloc(auth_length);
    strncpy(global_setting_auth_ptr, DEFAULT_AUTH, auth_length);

    int ip_length = strlen(DEFAULT_IP);
    global_setting_ip_ptr = (char*)malloc(ip_length);
    strncpy(global_setting_ip_ptr, DEFAULT_IP, ip_length);

    int path_for_data_length = strlen(DEFAULT_PATH_DATA);
    global_setting_path_for_data_ptr = (char*)malloc(path_for_data_length);
    strncpy(global_setting_path_for_data_ptr, DEFAULT_PATH_DATA,
            path_for_data_length);

    int path_for_crt_length = strlen(DEFAULT_PATH_CRT);
    global_setting_path_for_crt_ptr = (char*)malloc(path_for_crt_length);
    strncpy(global_setting_path_for_crt_ptr, DEFAULT_PATH_CRT,
            path_for_crt_length);

    int path_for_key_length = strlen(DEFAULT_PATH_KEY);
    global_setting_path_for_key_ptr = (char*)malloc(path_for_key_length);
    strncpy(global_setting_path_for_key_ptr, DEFAULT_PATH_KEY,
            path_for_key_length);

    int server_algorithm_length = strlen(DEFAULT_SERVER_ALGORITHM);
    global_setting_server_algorithm_ptr =
        (char*)malloc(server_algorithm_length);
    strncpy(global_setting_server_algorithm_ptr, DEFAULT_SERVER_ALGORITHM,
            server_algorithm_length);

    int server_protocol_length = strlen(DEFAULT_SERVER_PROTOCOL);
    global_setting_server_protocol_ptr = (char*)malloc(server_protocol_length);
    strncpy(global_setting_path_for_key_ptr, DEFAULT_SERVER_PROTOCOL,
            server_protocol_length);

    char default_settings_str[1024];
    snprintf(default_settings_str, sizeof(default_settings_str),
             "{\n  \"port\": %d,\n  \"auth\": \"%s\",\n  \"ip\": \"%s\",\n  "
             "\"path_for_data\": \"%s\"\n  \"path_for_crt\": \"%s\"\n  "
             "\"path_for_key\": \"%s\"\n  \"server_algorithm\": \"%s\"\n  "
             "\"server_protocol\": \"%s\"\n}",
             DEFAULT_PORT, DEFAULT_AUTH, DEFAULT_IP, DEFAULT_PATH_DATA,
             DEFAULT_PATH_CRT, DEFAULT_PATH_KEY, DEFAULT_SERVER_ALGORITHM,
             DEFAULT_SERVER_PROTOCOL);
    save_string_to_file(default_settings_str, settings_file_path);

    printf(
        "\n_database has automatically set default settings. Remember to "
        "check / update these before going to production! (see: %s)\n\n",
        settings_file_path);

    printf("\ndefaults: \n");
    printf("  port: %hd\n", global_setting_port);
    printf("  auth: %s\n", global_setting_auth_ptr);
    printf("  ip: %s\n", global_setting_ip_ptr);
    printf("  path_for_data: %s\n", global_setting_path_for_data_ptr);
    printf("  path_for_crt: %s\n", global_setting_path_for_data_ptr);
    printf("  path_for_key: %s\n", global_setting_path_for_data_ptr);
    printf("  server_algorithm: %s\n", global_setting_path_for_data_ptr);
    printf("  server_protocol: %s\n\n", global_setting_path_for_data_ptr);

    return 0;
  } else {
    printf("Settings error - file access issue: %s\n", file_access_issue);
    return 1;
  }
}

// This sets global_settings based on settings.conf content or default
// settings
int load_settings() {
  char* file_access_issue = check_file_access(settings_file_path, 1);
  if (file_access_issue == NULL) {
    int load_settings_status = load_settings_from_file();
    if (load_settings_status != 0) {
      return 1;
    }
  } else {
    int default_settings_status = intialize_default_settings(file_access_issue);
    if (default_settings_status != 0) {
      return 1;
    }
  }

  global_server_type = 0;

  if (strcmp(global_setting_server_algorithm_ptr, "blocking") == 0) {
    if (strcmp(global_setting_server_protocol_ptr, "http") == 0) {
      global_server_type = 11;
    } else if (strcmp(global_setting_server_protocol_ptr, "https") == 0) {
      global_server_type = 12;
    }
  } else if (strcmp(global_setting_server_algorithm_ptr, "event loop") == 0) {
    if (strcmp(global_setting_server_protocol_ptr, "http") == 0) {
      global_server_type = 21;
    } else if (strcmp(global_setting_server_protocol_ptr, "https") == 0) {
      global_server_type = 22;
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
  if (global_setting_path_for_data_ptr != NULL) {
    free(global_setting_path_for_data_ptr);
  }
  if (global_setting_path_for_crt_ptr != NULL) {
    free(global_setting_path_for_crt_ptr);
  }
  if (global_setting_path_for_key_ptr != NULL) {
    free(global_setting_path_for_key_ptr);
  }
  if (global_setting_server_algorithm_ptr != NULL) {
    free(global_setting_server_algorithm_ptr);
  }
  if (global_setting_server_protocol_ptr != NULL) {
    free(global_setting_server_protocol_ptr);
  }
}