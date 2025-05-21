#pragma once
#include <sys/socket.h>

typedef struct {
  int             socket;
  struct sockaddr addr;
} ctorm_conn_t;

char *ctorm_conn_ip(ctorm_conn_t *conn, char *buf);

#ifndef CTORM_EXPORT

#define ctorm_conn_recv(conn, buf, len, flags)                                 \
  recv((conn)->socket, buf, len, flags)
#define ctorm_conn_send(conn, buf, len, flags)                                 \
  send((conn)->socket, buf, len, flags)
void ctorm_conn_close(ctorm_conn_t *conn);

#endif
