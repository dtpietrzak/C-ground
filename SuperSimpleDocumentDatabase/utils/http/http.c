#include "http.h"

void validate_auth_header(const char *request_str,
                          sdb_http_response_t *http_response) {
  const char *auth_start = strstr(request_str, "Authorization: ");
  if (auth_start == NULL) {
    http_response->status = 401;
    s_set(&http_response->body, "\"Authorization header not found\"");
    return;
  }

  auth_start += strlen("Authorization: ");
  const char *auth_end = strchr(auth_start, '\n');
  if (auth_end == NULL) {
    auth_end = strchr(auth_start, '\r');  // Handle different line endings
  }
  if (auth_end == NULL) {
    http_response->status = 401;
    s_set(&http_response->body, "\"Auth header end of line not found\"");
    return;
  }

  // Calculate length of auth header
  size_t auth_len = auth_end - auth_start;

  // Allocate memory for auth header + null terminator
  char *auth_header = (char *)malloc(auth_len + 1);
  if (auth_header == NULL) {
    free(auth_header);
    http_response->status = 401;
    s_set(&http_response->body, "\"Memory allocation failed\"");
    return;
  }

  // Copy auth header and null terminate
  strncpy(auth_header, auth_start, auth_len);
  auth_header[auth_len] = '\0';

  // Remove carriage return if present
  if (auth_header[auth_len - 1] == '\r') {
    auth_header[auth_len - 1] = '\0';
    auth_len--;
  }

  if (auth_len == 0) {
    http_response->status = 401;
    s_set(&http_response->body,
          "\"Authorization header not found or invalid format\"");
  } else if (strcmp(auth_header, global_setting_auth_ptr)) {
    http_response->status = 401;
    s_set(&http_response->body, "\"Invalid authentication\"");
  } else if (auth_len < REQUIRED_AUTH_LENGTH) {
    http_response->status = 401;
    s_set(&http_response->body,
          "\"Invalid authentication header length, should be at least 32 "
          "characters\"");
  }

  free(auth_header);
}

// Validate the queries in the HTTP request
// If the queries are invalid, set the invalid message
// If the queries are valid, set the id and / or db name
//
// Returns the sdb_query_params_t struct
// if the queries are invalid, the invalid field will be set (not NULL)
sdb_query_params_t validate_and_parse_queries(sdb_http_request_t *http_request,
                                              const char *params[],
                                              int num_of_params) {
  // this is getting too large and should be refactored

  sdb_query_params_t sdb_query_params_t = {
      .invalid = NULL,
      .id = NULL,
      .key = NULL,
      .value = NULL,
      .query = NULL,
      .db = NULL,
      .limit = NULL,
      .offset = NULL,
      .sort = NULL,
  };

  if (http_request->num_queries > num_of_params) {
    sdb_query_params_t.invalid = "Invalid number of queries - too many";
    fprintf(stderr, "%s", sdb_query_params_t.invalid);
    return sdb_query_params_t;
  }

  bool id_is_required = false;
  bool key_is_required = false;
  bool value_is_required = false;
  bool query_is_required = false;
  bool db_is_required = false;

  for (int i = 0; i < http_request->num_queries; i++) {
    if (strcmp(params[i], "id") == 0) {
      id_is_required = true;
    }
    if (strcmp(params[i], "key") == 0) {
      key_is_required = true;
    }
    if (strcmp(params[i], "value") == 0) {
      value_is_required = true;
    }
    if (strcmp(params[i], "query") == 0) {
      query_is_required = true;
    }
    if (strcmp(params[i], "db") == 0) {
      db_is_required = true;
    }

    if (!strcmp(http_request->queries[i][0], "id")) {
      sdb_query_params_t.id = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "key")) {
      sdb_query_params_t.key = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "value")) {
      sdb_query_params_t.value = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "query")) {
      sdb_query_params_t.query = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "db")) {
      sdb_query_params_t.db = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "limit")) {
      sdb_query_params_t.limit = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "offset")) {
      sdb_query_params_t.offset = http_request->queries[i][1];
    }
    if (!strcmp(http_request->queries[i][0], "sort")) {
      sdb_query_params_t.sort = http_request->queries[i][1];
    }
  }

  if (id_is_required) {
    if (sdb_query_params_t.id == NULL) {
      sdb_query_params_t.invalid = "Id is missing";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    } else if (contains_invalid_chars(sdb_query_params_t.id,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      sdb_query_params_t.invalid = "Id contains invalid characters";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (key_is_required) {
    if (sdb_query_params_t.key == NULL) {
      sdb_query_params_t.invalid = "Key is missing";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    } else if (contains_invalid_chars(sdb_query_params_t.key,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      sdb_query_params_t.invalid = "Key contains invalid characters";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (value_is_required) {
    if (sdb_query_params_t.value == NULL) {
      sdb_query_params_t.invalid = "Value is missing";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    } else if (contains_invalid_chars(sdb_query_params_t.value,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      sdb_query_params_t.invalid = "Value contains invalid characters";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (query_is_required) {
    if (sdb_query_params_t.query == NULL) {
      sdb_query_params_t.invalid = "Query is missing";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    } else if (contains_invalid_chars(sdb_query_params_t.query,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      sdb_query_params_t.invalid = "Query contains invalid characters";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    } else if (strcmp(sdb_query_params_t.query, "all") != 0 &&
               strcmp(sdb_query_params_t.query, "gtr") != 0 &&
               strcmp(sdb_query_params_t.query, "lss") != 0 &&
               strcmp(sdb_query_params_t.query, "gte") != 0 &&
               strcmp(sdb_query_params_t.query, "lte") != 0) {
      sdb_query_params_t.invalid = "Query is invalid";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (db_is_required) {
    if (sdb_query_params_t.db == NULL) {
      sdb_query_params_t.invalid = "Db name is missing";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    } else if (contains_invalid_chars(sdb_query_params_t.db,
                                      INVALID_CHARS_DIRS_AND_FILES)) {
      sdb_query_params_t.invalid =
          "Db name contains invalid characters (cannot contain: "
          "\\/:*?\"<>|)";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
    if (contains_periods(sdb_query_params_t.db)) {
      sdb_query_params_t.invalid =
          "Db name contains invalid characters (cannot contain a period)";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (sdb_query_params_t.limit != NULL) {
    if (!contains_only_valid_chars(sdb_query_params_t.limit,
                                   VALID_CHARS_NUMBERS)) {
      sdb_query_params_t.invalid =
          "Limit contains invalid characters - must be a positive integer";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (sdb_query_params_t.offset != NULL) {
    if (!contains_only_valid_chars(sdb_query_params_t.offset,
                                   VALID_CHARS_NUMBERS)) {
      sdb_query_params_t.invalid =
          "Offset contains invalid characters - must be a positive integer";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (sdb_query_params_t.sort != NULL) {
    if (strcmp(sdb_query_params_t.sort, "asc") != 0 &&
        strcmp(sdb_query_params_t.sort, "desc") != 0) {
      sdb_query_params_t.invalid =
          "Sort contains invalid characters - must be 'asc' "
          "or 'desc'";
      fprintf(stderr, "%s", sdb_query_params_t.invalid);
      return sdb_query_params_t;
    }
  }

  if (sdb_query_params_t.invalid) {
    printf("Unexpected query invalid error: %s\n", sdb_query_params_t.invalid);
  }

  return sdb_query_params_t;
}

void parse_http_request(const char *request_str, sdb_http_request_t *request) {
  // Parse method and path
  char format_str[32];
  snprintf(format_str, sizeof(format_str), "%%%ds %%%ds",
           MAX_REQ_METHOD_SIZE - 1, MAX_REQ_PATH_SIZE - 1);
  sscanf(request_str, format_str, request->method, request->path);

  // Check if there are query parameters in the path
  char *query_start = strchr(request->path, '?');
  if (query_start != NULL) {
    *query_start = '\0';  // Separate path and query parameters
    query_start++;        // Move past the '?'

    // Parse query parameters
    const char *param_pair = strtok(query_start, "&");
    int query_index = 0;
    while (param_pair != NULL && query_index < MAX_REQ_QUERIES) {
      char *equal_sign = strchr(param_pair, '=');
      if (equal_sign != NULL) {
        *equal_sign = '\0';
        equal_sign++;

        // id
        strncpy(request->queries[query_index][0], param_pair,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][0][MAX_REQ_QUERY_SIZE - 1] = '\0';

        // value
        strncpy(request->queries[query_index][1], equal_sign,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][1][MAX_REQ_QUERY_SIZE - 1] = '\0';
      } else {
        // No '=' found, treat as id with empty value
        strncpy(request->queries[query_index][0], param_pair,
                MAX_REQ_QUERY_SIZE - 1);
        request->queries[query_index][0][MAX_REQ_QUERY_SIZE - 1] = '\0';
        request->queries[query_index][1][0] = '\0';  // Empty value
      }
      query_index++;
      param_pair = strtok(NULL, "&");
    }
    request->num_queries = query_index;
  }

  // Parse headers
  const char *body_start = strstr(request_str, "\r\n\r\n");
  if (body_start != NULL) {
    // Copy headers
    const char *header_end = strstr(request_str, "\r\n");
    while (header_end != NULL && header_end < body_start) {
      header_end += 2;  // Move past "\r\n"
      const char *header_start = header_end;
      header_end = strstr(header_start, "\r\n");
      if (header_end != NULL) {
        if (request->num_headers < MAX_REQ_HEADERS) {
          int header_len = header_end - header_start;
          strncpy(request->headers[request->num_headers], header_start,
                  header_len);
          request->headers[request->num_headers][header_len] = '\0';
          request->num_headers++;
        }
      }
    }
    s_set(&request->body, body_start + 4);
  }
}

void compile_http_response(sdb_http_response_t *http_response,
                           SString *response_str) {
  // start status_code
  SString status_code;
  s_init(&status_code, "", 36);

  switch (http_response->status) {
    case 200:
      s_set(&status_code, "200 OK");
      break;
    case 201:
      s_set(&status_code, "201 Created");
      break;
    case 204:
      s_set(&status_code, "204 No Content");
      break;
    case 400:
      s_set(&status_code, "400 Bad Request");
      break;
    case 401:
      s_set(&status_code, "401 Unauthorized");
      break;
    case 404:
      s_set(&status_code, "404 Not Found");
      break;
    default:
      s_set(&status_code, "500 Internal Server Error");
      break;
  }

  SString response_content_str;
  s_init(&response_content_str, "", MAX_RES_SIZE);
  s_compile(&response_content_str, "{\"status\":\"%d\",\"body\":%s}",
            http_response->status, http_response->body.value);

  s_compile(response_str,
            "HTTP/1.1 %s\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            status_code.value, response_content_str.length,
            response_content_str.value);
}

// url encoding function
// https://www.urlencoder.io/learn/
// https://www.w3schools.com/tags/ref_urlencode.ASP

char *url_encode(const char *str) {
  const char *pstr = str;
  char *buf = (char *)malloc(strlen(str) * 3 + 1);
  char *pbuf = buf;

  if (buf == NULL) {
    return NULL;  // Check for successful allocation
  }

  while (*pstr) {
    if (isalnum((unsigned char)*pstr) || *pstr == '-' || *pstr == '_' ||
        *pstr == '.' || *pstr == '~') {
      *pbuf++ = *pstr;
    } else if (*pstr == ' ') {
      *pbuf++ = '+';
    } else {
      snprintf(pbuf, 4, "%%%02X", (unsigned char)*pstr);
      pbuf += 3;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

// url decoding function
char *url_decode(const char *str) {
  const char *pstr = str;
  char *buf = (char *)malloc(strlen(str) + 1);
  char *pbuf = buf;

  if (buf == NULL) {
    return NULL;  // Check for successful allocation
  }

  while (*pstr) {
    if (*pstr == '+') {
      *pbuf++ = ' ';
    } else if (*pstr == '%' && isxdigit(pstr[1]) && isxdigit(pstr[2])) {
      const char hex[3] = {pstr[1], pstr[2], 0};
      *pbuf++ = (char)strtol(hex, NULL, 16);
      pstr += 2;
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}