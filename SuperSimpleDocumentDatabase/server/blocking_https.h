#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../requests/_request_distributor/request_distributor.h"
#include "../utils/sstring.h"

int start_server_blocking_https(int port);