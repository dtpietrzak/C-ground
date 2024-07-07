#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define MAX_HEADERS 10  // Adjust this based on your expected number of headers

typedef struct {
  char method[16];  // Adjust size based on expected method length (e.g., GET,
                    // POST)
  char path[256];   // Adjust size based on expected path length
  char headers[MAX_HEADERS][256];  // Adjust size based on expected header
                                   // length and number of headers
  int num_headers;
  char body[1024];  // Adjust size based on expected body length
} HttpRequest;

char *create_http_header(int status_code, const char *content_type,
                         int content_length) {
  // Current time
  time_t now;
  time(&now);
  struct tm *gmt = gmtime(&now);
  char date_str[64];
  strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S GMT", gmt);

  // Allocate memory for the response
  char *response =
      (char *)malloc(1024 * sizeof(char));  // Adjust size as needed
  if (response == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return NULL;
  }
  response[0] = '\0';  // Ensure the string is empty initially

  // Format HTTP status line
  sprintf(response, "HTTP/1.1 %d %s\r\n", status_code,
          status_code == 200 ? "OK" : "Unknown Status");

  // Append headers
  sprintf(response + strlen(response), "Content-Type: %s\r\n", content_type);
  sprintf(response + strlen(response), "Content-Length: %d\r\n",
          content_length);
  // sprintf(response + strlen(response), "Date: %s\r\n", date_str);
  // sprintf(response + strlen(response), "Server: IndexStar\r\n");

  // End of headers
  strcat(response, "\r\n");

  return response;
}

void parse_http_request(const char *request_str, HttpRequest *request) {
  // Initialize request struct
  memset(request, 0, sizeof(HttpRequest));

  // Parse method and path
  sscanf(request_str, "%s %s", request->method, request->path);

  // Parse headers and body
  const char *body_start = strstr(request_str, "\r\n\r\n");
  if (body_start != NULL) {
    // Copy headers
    const char *header_end = strstr(request_str, "\r\n");
    while (header_end != NULL && header_end < body_start) {
      header_end += 2;  // Move past "\r\n"
      const char *header_start = header_end;
      header_end = strstr(header_start, "\r\n");
      if (header_end != NULL) {
        int header_len = header_end - header_start;
        if (request->num_headers < MAX_HEADERS) {
          strncpy(request->headers[request->num_headers], header_start,
                  header_len);
          request->headers[request->num_headers][header_len] = '\0';
          request->num_headers++;
        }
      }
    }

    // Copy body
    if (body_start != NULL) {
      strcpy(request->body, body_start + 4);  // Move past "\r\n\r\n"
    }
  }
}

// Function to handle processing of received data
const char *process_request(const char *request_str) {
  HttpRequest request;
  parse_http_request(request_str, &request);

  char message[512];  // Adjust size as needed
  sprintf(message, "{\"path\": \"%s\", \"body\": \"%s\"}", request.path,
          request.body);

  int status_code = 200;
  const char *content_type = "application/json";
  int content_length = strlen(message);

  char *http_response =
      create_http_header(status_code, content_type, content_length);
  if (http_response == NULL) {
    return NULL;  // Error handling for memory allocation failure
  }

  strcat(http_response, message);
  memset(message, 0, sizeof(message));

  return http_response;
}

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    printf("Received data: %.*s\n", (int)nread, buf->base);

    // Process the received data
    const char *processed_data = process_request(buf->base);

    printf("Returned data: %.*s\n", (int)nread, processed_data);

    uv_write_t write_req;
    uv_buf_t write_buf =
        uv_buf_init((char *)processed_data, strlen(processed_data));
    uv_write(&write_req, client_stream, &write_buf, 1, NULL);
  } else if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
    }
    uv_close((uv_handle_t *)client_stream, NULL);
  }

  // Free buffer allocated by on_alloc_buffer
  if (buf->base) {
    free(buf->base);
  }
}

void on_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  if (!client) {
    fprintf(stderr, "Memory allocation error\n");
    return;
  }

  uv_tcp_init(uv_default_loop(), client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    printf("Accepted connection!\n");
    uv_read_start((uv_stream_t *)client, on_alloc_buffer, on_read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

int main() {
  uv_tcp_t server;
  uv_loop_t *loop = uv_default_loop();

  uv_tcp_init(loop, &server);

  struct sockaddr_in bind_addr;
  uv_ip4_addr("127.0.0.1", 7777, &bind_addr);

  uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);
  int r = uv_listen((uv_stream_t *)&server, SOMAXCONN, on_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }

  printf("Listening on port 7777\n");
  uv_run(loop, UV_RUN_DEFAULT);

  return 0;
}
