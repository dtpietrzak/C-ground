#include "../../utils/parson.h"
#include "../../utils/request_operations.h"
#include "index_utils.h"

const char* index_distributor(HttpResponse* http_response,
                              JSON_Value_Type json_value_type,
                              const JSON_Object* json_object, const char* key);

