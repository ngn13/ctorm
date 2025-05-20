#include "socket.h"
#include "error.h"

#include "conn.h"
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
  bool             ret = false;

  ctorm_uri_init(&uri);

  if (NULL == ctorm_uri_parse_host(&uri, addr))
    return false; // errno set by ctorm_uri_parse_host()

  // resolve the host name
  if (getaddrinfo(uri.host, NULL, NULL, &hostinfo) != 0 || NULL == info) {
    ctorm_uri_free(&uri);
    return false; // errno set by getaddrinfo()
  }

  for (cur = hostinfo; NULL != cur; cur = cur->ai_next) {
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
  if (NULL != cur) {
    memcpy(info, cur, sizeof(*cur));
    ret = true;
  }

  // free the host addrinfo
  freeaddrinfo(hostinfo);
  ctorm_uri_free(&uri);

  return ret;
}

bool ctorm_socket_set_opts(ctorm_app_t *app, int sockfd) {
  struct timeval timeout;
  int            flag = 1;

  // clear the timeout structure
  memset(&timeout, 0, sizeof(timeout));

#ifdef TCP_QUICKACK
  /*

   * TCP delayed acknowledgment, buffers and combines multiple ACKs to reduce
   * overhead it may delay the ACK response by up to 500ms, and we don't want
   * that because slow bad fast good

  */
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0) {
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    return false;
  }
#endif

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
  int             ssock = -1, csock = -1, flag = 1;
  struct sockaddr caddr;
  struct addrinfo info;
  socklen_t       len = 0;
  bool            ret = false;

  // parse the host to get the addrinfo structure
  if (!ctorm_socket_resolve(addr, &info)) {
    debug("failed to resolve the address: %s", ctorm_error());
    ctorm_error_set(app, CTORM_ERR_RESOLVE_FAIL);
    goto end;
  }

  // create a new TCP socket
  if ((ssock = socket(info.ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debug("failed to create socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_SOCKET_FAIL);
    goto end;
  }

  debug("created socket %d for %p", ssock, app);

  // prevent EADDRINUSE
  if (setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    debug("failed to set the REUSEADDR: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_SOCKET_OPT_FAIL);
    goto end;
  }

  // bind and listen on the provided host
  if (bind(ssock, info.ai_addr, info.ai_addrlen) < 0) {
    debug("failed to bind the socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_BIND_FAIL);
    goto end;
  }

  if (listen(ssock, app->config->max_connections) < 0) {
    debug("failed to listen socket: %s", strerror(errno));
    ctorm_error_set(app, CTORM_ERR_LISTEN_FAIL);
    goto end;
  }

  // new connection handler loop
  while (app->running) {
    if ((csock = accept(ssock, &caddr, &len)) < 0) {
      if (errno == EINTR) {
        debug("accept got interrupted");
        break;
      }

      debug("failed to accept new connection: %s", strerror(errno));
      goto end;
    }

    debug("new connection: %d", csock);

    if (!ctorm_socket_set_opts(app, csock)) {
      debug("setsockopt failed for %d: %s", csock, strerror(errno));
      errno = CTORM_ERR_SOCKET_OPT_FAIL;
      goto end;
    }

    debug("creating a new connection for %d", csock);

    if (!ctorm_conn_new(app, &caddr, csock)) {
      debug("failed to create a new connection");
      goto end;
    }

    // clear client connection info
    memset(&caddr, 0, sizeof(caddr));
    csock = -1;
  }

  // app is no longer running
  debug("stopping the connection handler");
  ctorm_error_clear(app);
  ret = true;

end:
  // close the server socket
  if (ssock != -1)
    close(ssock);

  // close the recent client socket
  if (csock != -1)
    close(csock);

  return ret;
}
