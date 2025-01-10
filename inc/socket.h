#pragma once
#include "app.h"

#include <stdbool.h>
#include <stdint.h>

bool socket_set_opts(ctorm_app_t *app, int sockfd);
bool socket_start(ctorm_app_t *app, char *addr, uint16_t port);
