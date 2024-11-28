#pragma once
#include "ctorm.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>

bool socket_set_opts(app_t *app, int sockfd);
bool socket_start(app_t *app, char *addr, uint16_t port);
