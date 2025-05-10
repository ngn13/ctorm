#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  void           *app;
  struct sockaddr addr;
  int             socket;
} ctorm_conn_t;

#ifndef CTORM_EXPORT

ctorm_conn_t *ctorm_conn_new();
void          ctorm_conn_free(ctorm_conn_t *con);

void ctorm_conn_handle(ctorm_conn_t *con);
#define ctorm_conn_recv(c, b, s, f) recv(c->socket, b, s, f)
#define ctorm_conn_send(c, b, s, f) send(c->socket, b, s, f)

#endif
