#pragma once
#include "app.h"

#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>

bool socket_parse_host(const char *host, struct addrinfo *info);
bool socket_set_opts(ctorm_app_t *app, int sockfd);
bool socket_start(ctorm_app_t *app, const char *addr);
