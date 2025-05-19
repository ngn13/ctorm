#pragma once
#ifndef CTORM_EXPORT

#include "app.h"
#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>

bool ctorm_socket_resolve(char *addr, struct addrinfo *info);
bool ctorm_socket_set_opts(ctorm_app_t *app, int sockfd);
bool ctorm_socket_start(ctorm_app_t *app, char *addr);

#endif
