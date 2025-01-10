#include "connection.h"
#include "socket.h"
#include "pool.h"
#include "log.h"

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

bool socket_set_opts(ctorm_app_t *app, int sockfd) {
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

bool socket_start(ctorm_app_t *app, char *addr, uint16_t port) {
  int              sockfd = -1, flag = 1;
  struct addrinfo *ainfo = NULL, *cur = NULL;
  socklen_t        addr_len = 0;
  bool             ret      = false;
  connection_t    *con      = NULL;

  if (0 == port)
    goto end;

  if (getaddrinfo(addr, NULL, NULL, &ainfo) != 0 || NULL == ainfo)
    goto end;

  for (cur = ainfo; cur != NULL; cur = cur->ai_next) {
    switch (cur->ai_family) {
    case AF_INET:
      ((struct sockaddr_in *)cur->ai_addr)->sin_port = htons(port);
      goto found_addr;

    case AF_INET6:
      ((struct sockaddr_in6 *)cur->ai_addr)->sin6_port = htons(port);
      goto found_addr;
    }
  }

  debug("failed to resolve the address");
  goto end;

found_addr:
  if ((sockfd = socket(cur->ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debug("failed to create socket: %s", strerror(errno));
    goto end;
  }

  // prevent EADDRINUSE
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    debug("failed to set the REUSEADDR: %s", strerror(errno));
    goto end;
  }

  if (bind(sockfd, cur->ai_addr, cur->ai_addrlen) != 0) {
    debug("failed to bind the socket: %s", strerror(errno));
    goto end;
  }

  if (listen(sockfd, app->config->max_connections) != 0) {
    debug("failed to listen socket: %s", strerror(errno));
    goto end;
  }

  // start the server
  if (!app->config->disable_startup)
    info("starting the application on %s:%u", addr, port);

  // new connection handler loop
  do {
    if (NULL == con)
      goto con_new;

    debug("accepted a new connection (con: %p, socket %d)", con, con->socket);

    if (!socket_set_opts(app, con->socket)) {
      error("failed to setsockopt for connection (con: %p, socket %d): %s", con, con->socket, strerror(errno));
      break;
    }

    con->app = app;

    debug("creating a thread for connection (con: %p, socket %d)", con, con->socket);
    pool_add(app->pool, (void *)connection_handle, (void *)con);

  con_new:
    if ((con = connection_new()) == NULL) {
      debug("failed to create a new connection: %s", app_geterror());
      goto end;
    }
  } while (app->running && (con->socket = accept(sockfd, &con->addr, &addr_len)) > 0);

  ret = true;
end:
  if (NULL != ainfo)
    freeaddrinfo(ainfo);

  if (sockfd > 0)
    close(sockfd);

  if (NULL != con) {
    debug("freeing unused connection (%p)", con);
    connection_free(con);
  }

  return ret;
}
