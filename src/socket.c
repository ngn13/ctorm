#include "socket.h"
#include "error.h"

#include "conn.h"
#include "pool.h"
#include "uri.h"
#include "util.h"
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

bool ctorm_socket_resolve(char *addr, struct addrinfo *info) {
  struct addrinfo *hostinfo = NULL, *cur = NULL;
  ctorm_uri_t      uri;

  ctorm_uri_init(&uri);

  if (NULL == ctorm_uri_parse_host(&uri, addr))
    return false; // errno set by ctorm_uri_parse_host()

  // resolve the host name
  if (getaddrinfo(uri.host, NULL, NULL, &hostinfo) != 0 || NULL == info) {
    ctorm_uri_free(&uri);
    return false; // errno set by getaddrinfo()
  }

  for (cur = hostinfo; cur != NULL; cur = cur->ai_next) {
    if (AF_INET == cur->ai_family) {
      ((struct sockaddr_in *)cur->ai_addr)->sin_port = htons(uri.port);
      break;
    }

    else if (AF_INET6 == cur->ai_family) {
      ((struct sockaddr_in6 *)cur->ai_addr)->sin6_port = htons(uri.port);
      break;
    }
  }

  // copy the addrinfo for the host to the provided info structure
  if (NULL != cur)
    memcpy(info, cur, sizeof(*cur));

  // free the host addrinfo
  freeaddrinfo(hostinfo);
  ctorm_uri_free(&uri);

  return true;
}

bool ctorm_socket_set_opts(ctorm_app_t *app, int sockfd) {
  struct timeval timeout;
  bzero(&timeout, sizeof(timeout));
  int flag = 1;

  /*

   * TCP delayed acknowledgment, buffers and combines multiple ACKs to reduce
   * overhead it may delay the ACK response by up to 500ms, and we don't want
   * that because slow bad fast good

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0) {
    errno = CTORM_ERR_SOCKET_OPT_FAIL;
    return false;
  }

  /*

   * Nagle's algorithm buffers and combines outgoing packets to solve the
   * "small-packet problem", we want to send all the packets as fast as possible
   * so we can disable this buffering with TCP_NODELAY

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
    errno = CTORM_ERR_SOCKET_OPT_FAIL;
    return false;
  }

  // set the socket timeout
  if (app->config->tcp_timeout > 0) {
    timeout.tv_sec  = app->config->tcp_timeout;
    timeout.tv_usec = 0;

    setsockopt(
        sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  }

  // make the socket blocking
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & (~O_NONBLOCK)) == -1) {
    errno = CTORM_ERR_FCNTL_FAIL;
    return false;
  }

  return true;
}

bool ctorm_socket_start(ctorm_app_t *app, char *addr) {
  struct addrinfo info;
  socklen_t       addrlen = 0;
  int             sockfd = -1, flag = 1;
  ctorm_conn_t   *con = NULL;
  bool            ret = false;

  // parse the host to get the addrinfo structure
  if (!ctorm_socket_resolve(addr, &info)) {
    debug("failed to resolve the address: %s", ctorm_error());
    goto end;
  }

  // create a new TCP socket
  if ((sockfd = socket(info.ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debug("failed to create socket: %s", strerror(errno));
    goto end;
  }

  // prevent EADDRINUSE
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    debug("failed to set the REUSEADDR: %s", strerror(errno));
    errno = CTORM_ERR_SOCKET_OPT_FAIL;
    goto end;
  }

  // bind and listen on the provided host
  if (bind(sockfd, info.ai_addr, info.ai_addrlen) != 0) {
    debug("failed to bind the socket: %s", strerror(errno));
    goto end;
  }

  if (listen(sockfd, app->config->max_connections) != 0) {
    debug("failed to listen socket: %s", strerror(errno));
    goto end;
  }

  // new connection handler loop
  while (app->running) {
    if (con == NULL && (con = ctorm_conn_new()) == NULL) {
      debug("failed to create a new connection: %s", ctorm_error());
      goto end;
    }

    addrlen  = sizeof(con->addr);
    con->app = app;

    if ((con->socket = accept(sockfd, &con->addr, &addrlen)) <= 0) {
      if (errno == EINTR) {
        debug("accept got interrupted");
        break;
      }

      debug("failed to accept new connection: %s", strerror(errno));
      goto end;
    }

    debug("accepted a new connection (con: %p, socket %d)", con, con->socket);

    if (!ctorm_socket_set_opts(app, con->socket)) {
      error("failed to setsockopt for connection (con: %p, socket %d): %s",
          con,
          con->socket,
          strerror(errno));
      errno = CTORM_ERR_SOCKET_OPT_FAIL;
      goto end;
    }

    debug("creating a thread for connection (con: %p, socket %d)",
        con,
        con->socket);
    ctorm_pool_add(app->pool, (func_t)ctorm_conn_handle, (void *)con);

    con = NULL;
  }

  // app is no longer running
  debug("stopping the connection handler");
  ret = true;

end:
  // close the socket
  if (sockfd > 0)
    close(sockfd);

  // free the unused connection structure
  if (NULL != con) {
    debug("freeing unused connection (%p)", con);
    ctorm_conn_free(con);
  }

  return ret;
}
