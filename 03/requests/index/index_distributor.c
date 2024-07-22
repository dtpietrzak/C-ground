#include "index_distributor.h"

const char* index_distributor(HttpResponse* http_response,
                              JSON_Value_Type json_value_type,
                              const JSON_Object* json_object, const char* key) {
  switch (json_value_type) {
    case JSONString: {
      // index the document
      const char* document_dot_value =
          json_object_dotget_string(json_object, key);
      if (document_dot_value == NULL) {
        http_response->status = 500;
        s_compile(
            &http_response->body,
            "Failed to get index value from document by key: '%s' - document "
            "value for this key must be a string",
            key);
        return NULL;
      }

      return document_dot_value;
    }
    default:
      http_response->status = 500;
      s_compile(
          &http_response->body,
          "Failed to get index value from document by key: '%s' - document "
          "value for this key must be a string",
          key);
      return NULL;
  }
}