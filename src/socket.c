#include "../include/connection.h"
#include "../include/socket.h"
#include "../include/errors.h"
#include "../include/pool.h"

#include "../include/log.h"
#include "../include/req.h"
#include "../include/res.h"

#include <netinet/tcp.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

bool socket_set_opts(app_t *app, int sockfd) {
  struct timeval timeout;
  bzero(&timeout, sizeof(timeout));
  int flag = 1;

  /*

   * TCP delayed acknowledgment, buffers and combines multiple ACKs to reduce overhead
   * it may delay the ACK response by up to 500ms, and we don't want that because slow bad fast good

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0)
    return false;

  /*

   * nagle's algorithm buffers and combines outgoing packets to solve the "small-packet problem",
   * we want to send all the packets as fast as possible so we can disable buffering with TCP_NODELAY

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0)
    return false;

  // set the socket timeout
  if (app->config->tcp_timeout > 0) {
    timeout.tv_sec  = app->config->tcp_timeout;
    timeout.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  }

  // make the socket blocking
  fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & (~O_NONBLOCK));
  return true;
}

bool socket_start(app_t *app, char *addr, uint16_t port) {
  int              sockfd = -1, flag = 1;
  struct addrinfo *info = NULL, *cur = NULL;
  struct sockaddr  saddr;
  socklen_t        saddr_len = sizeof(saddr);
  bool             ret       = false;
  connection_t    *con       = NULL;

  bzero(&saddr, sizeof(saddr));

  if (0 == port)
    goto end;

  if (getaddrinfo(addr, NULL, NULL, &info) != 0 || NULL == info)
    goto end;

  for (cur = info; cur != NULL; cur = cur->ai_next) {
    switch (cur->ai_family) {
    case AF_INET:
      ((struct sockaddr_in *)cur->ai_addr)->sin_port = htons(port);
      goto found_addr;

    case AF_INET6:
      ((struct sockaddr_in6 *)cur->ai_addr)->sin6_port = htons(port);
      goto found_addr;
    }
  }

  debug("Failed to resolve the address");
  goto end;

found_addr:
  memcpy(&saddr, cur->ai_addr, sizeof(saddr));

  if ((sockfd = socket(saddr.sa_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debug("Failed to create socket: %s", strerror(errno));
    goto end;
  }

  // prevent EADDRINUSE
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    debug("Failed to set the REUSEADDR: %s", strerror(errno));
    goto end;
  }

  if (bind(sockfd, &saddr, sizeof(saddr)) != 0) {
    debug("Failed to bind the socket: %s", strerror(errno));
    goto end;
  }

  if (listen(sockfd, app->config->max_connections) != 0) {
    debug("Failed to listen socket: %s", strerror(errno));
    goto end;
  }

  // new connection handler loop
  do {
    if (NULL == con)
      goto con_new;

    debug("Accepted a new connection (con: %p, socket %d)", con, con->socket);

    if (!socket_set_opts(app, con->socket)) {
      error("Failed to setsockopt for connection (con: %p, socket %d): %s", con, con->socket, strerror(errno));
      break;
    }

    con->app = app;

    debug("Creating a thread for connection (con: %p, socket %d)", con, con->socket);
    pool_add(app->pool, (void *)connection_handle, (void *)con);

  con_new:
    if ((con = connection_new()) == NULL) {
      debug("Failed to create a new connection: %s", app_geterror());
      goto end;
    }
  } while (app->running && (con->socket = accept(sockfd, &con->addr, &saddr_len)) > 0);

  ret = true;
end:
  if (NULL != info)
    freeaddrinfo(info);

  if (sockfd > 0)
    close(sockfd);

  if (NULL != con) {
    debug("Freeing unused connection (%p)", con);
    connection_free(con);
  }

  return ret;
}
