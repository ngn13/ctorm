#pragma once
#ifndef CTORM_EXPORT
#include <sys/socket.h>
#include "app.h"

bool ctorm_conn_new(ctorm_app_t *app, struct sockaddr *addr, int socket);

#endif
