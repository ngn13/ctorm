#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  void           *app;
  struct sockaddr addr;
  int             socket;
} connection_t;

#ifndef CTORM_EXPORT

connection_t *connection_new();
void          connection_free(connection_t *con);

void connection_handle(connection_t *con);
#define connection_recv(c, b, s, f) recv(c->socket, b, s, f)
#define connection_send(c, b, s, f) send(c->socket, b, s, f)

#endif
