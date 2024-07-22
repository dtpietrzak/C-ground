#include "../../utils/parson.h"
#include "../../utils/request_operations.h"
#include "index_utils.h"

int index_a_doc(HttpResponse* http_response, const char* db_name,
                const char* filename, const char* request_meta_string);