#include <stdint.h>

// ptrs
char *global_setting_auth_ptr;
char *global_setting_ip_ptr;
char *global_setting_path_for_data_ptr;
char *global_setting_path_for_crt_ptr;
char *global_setting_path_for_key_ptr;
char *global_setting_server_algorithm_ptr;
char *global_setting_server_protocol_ptr;

// not-ptrs
int16_t global_setting_port;
int global_max_req_size;
int global_max_res_size;
int global_server_type; // 11, 12, 21, 22