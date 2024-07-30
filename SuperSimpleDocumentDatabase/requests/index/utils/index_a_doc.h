#include <sdb_utils.h>
#include "../index_distributor/index_distributor.h"
#include "index_utils.h"

int index_a_doc(sdb_http_response_t* http_response, const char* db_name,
                const char* filename, const char* request_meta_string);