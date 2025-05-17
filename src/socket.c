#include "socket.h"
#include "error.h"

#include "conn.h"
#include "pool.h"
#include "uri.h"
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
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    return false;
  }

  /*

   * Nagle's algorithm buffers and combines outgoing packets to solve the
   * "small-packet problem", we want to send all the packets as fast as possible
   * so we can disable this buffering with TCP_NODELAY

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
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
  ctorm_conn_t   *con = NULL;
  struct addrinfo info;
  socklen_t       len  = 0;
  int             flag = 1, sock = -1;
  bool            ret = false;

  // parse the host to get the addrinfo structure
  if (!ctorm_socket_resolve(addr, &info)) {
    debug("failed to resolve the address: %s", ctorm_error());
    ctorm_error_set(app, CTORM_ERR_RESOLVE_FAIL);
    goto end;
  }

  // create a new TCP socket
  if ((sock = socket(info.ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debug("failed to create socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_SOCKET_FAIL);
    goto end;
  }

  debug("created socket %d for %p", sock, app);

  // prevent EADDRINUSE
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    debug("failed to set the REUSEADDR: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    goto end;
  }

  // bind and listen on the provided host
  if (bind(sock, info.ai_addr, info.ai_addrlen) < 0) {
    debug("failed to bind the socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_BIND_FAIL);
    goto end;
  }

  if (listen(sock, app->config->max_connections) < 0) {
    debug("failed to listen socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_LISTEN_FAIL);
    goto end;
  }

  // new connection handler loop
  while (app->running) {
    if (NULL == con && (con = ctorm_conn_new()) == NULL) {
      debug("failed to create a new connection: %s", ctorm_error());
      goto end; // errno set by ctorm_conn_new
    }

    len      = sizeof(con->addr);
    con->app = app;

    if ((con->socket = accept(sock, &con->addr, &len)) <= 0) {
      if (errno == EINTR) {
        debug("accept got interrupted");
        break;
      }

      debug("failed to accept new connection: %s", strerror(errno));
      goto end;
    }

    debug("accepted new connection with socket %d: %p", con->socket, con);

    if (!ctorm_socket_set_opts(app, con->socket)) {
      debug("setsockopt failed for %p: %s", con, strerror(errno));
      errno = CTORM_ERR_SOCKET_OPT_FAIL;
      goto end;
    }

    debug("adding %p to the thread pool", con);
    ctorm_pool_add(
        app->pool, (ctorm_work_func_t)ctorm_conn_handle, (void *)con);

    con = NULL;
  }

  // app is no longer running
  debug("stopping the connection handler");
  ctorm_error_clear(app);
  ret = true;

end:
  // close the server socket
  if (sock != -1)
    close(sock);

  // free the unused connection structure
  if (NULL != con)
    ctorm_conn_free(con);

  return ret;
}
