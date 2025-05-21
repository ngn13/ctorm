#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "conn.h"
#include "log.h"

char *ctorm_conn_ip(ctorm_conn_t *conn, char *buf) {
  if (NULL == buf)
    buf = calloc(1, INET6_ADDRSTRLEN + 1);

  if (NULL == buf) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return NULL;
  }

  switch (conn->addr.sa_family) {
  case AF_INET:
    inet_ntop(AF_INET,
        &((struct sockaddr_in *)&conn->addr)->sin_addr,
        buf,
        INET_ADDRSTRLEN);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6,
        &((struct sockaddr_in6 *)&conn->addr)->sin6_addr,
        buf,
        INET6_ADDRSTRLEN);
    break;
  }

  return buf;
}

void ctorm_conn_close(ctorm_conn_t *conn) {
  if (NULL != conn && conn->socket > 0)
    close(conn->socket);
}
