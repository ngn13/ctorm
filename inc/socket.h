#pragma once
#include "app.h"

#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>

bool ctorm_socket_parse_host(const char *host, struct addrinfo *info);
bool ctorm_socket_set_opts(ctorm_app_t *app, int sockfd);
bool ctorm_socket_start(ctorm_app_t *app, const char *addr);
