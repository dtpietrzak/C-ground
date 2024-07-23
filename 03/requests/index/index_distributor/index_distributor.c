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
    case JSONBoolean: {
      // index the document
      const JSON_Value* json_value = json_object_get_value(json_object, key);
      if (json_value == NULL) {
        http_response->status = 500;
        s_compile(
            &http_response->body,
            "Failed to get index value from document by key: '%s' - document "
            "value for this key must be a boolean",
            key);
        return NULL;
      }

      return json_value_get_boolean(json_value) ? "true" : "false";
    }
    case JSONNumber: {
      // index the document
      JSON_Value* json_value = json_object_get_value(json_object, key);
      if (json_value == NULL) {
        http_response->status = 500;
        s_compile(
            &http_response->body,
            "Failed to get index value from document by key: '%s' - document "
            "value for this key must be a number",
            key);
        return NULL;
      }

      const double number = json_value_get_number(json_value);
      char temp_str[50];
      intToStr(number, temp_str);

      char* str = (char*)malloc((strlen(temp_str) + 1) * sizeof(char));
      if (str == NULL) {
        http_response->status = 500;
        s_compile(
            &http_response->body,
            "Memory allocation failed for the string conversion of key: '%s'",
            key);
        return NULL;
      }

      strcpy(str, temp_str);

      return str;
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