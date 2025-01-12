#include "connection.h"
#include "socket.h"
#include "errors.h"
#include "pool.h"
#include "log.h"
#include "util.h"

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

bool socket_parse_host(const char *host, struct addrinfo *info) {
  bool     is_reading_ipv6 = false, is_reading_port = false;
  char     hostname[UINT8_MAX + 1], hostport[6];
  uint16_t indx = 0;

  // clear the buffers to make sure everything is NULL terminated
  bzero(hostname, sizeof(hostname));
  bzero(hostport, sizeof(hostport));

  for (; *host != 0; host++) {
    if (*host == '[' && !is_reading_ipv6) {
      // [::1]:80
      // ^
      is_reading_ipv6 = true;
      continue;
    }

    if (*host == ']' && is_reading_ipv6) {
      // [::1]:80
      //     ^
      is_reading_ipv6 = false;
      continue;
    }

    if (is_reading_port && !is_digit(*host)) {
      errno = BadPort;
      return false;
    }

    if (!is_reading_ipv6 && *host == ':') {
      // [::1]:80
      //      ^
      is_reading_port = true;
      indx            = 0;
      continue;
    }

    if (is_reading_port && indx >= sizeof(hostport)) {
      errno = PortTooLarge;
      return false;
    }

    else if (!is_reading_port && indx >= sizeof(hostname)) {
      errno = NameTooLarge;
      return false;
    }

    if (is_reading_port)
      hostport[indx++] = *host;
    else
      hostname[indx++] = *host;
  }

  if (*hostname == 0) {
    errno = BadName;
    return false;
  }

  debug("name: %s port: %s", hostname, hostport);

  struct addrinfo *hostinfo = NULL, *cur = NULL;
  int              port = 0;

  // convert host port to integer
  if (*hostport != 0 && (port = atoi(hostport)) <= 0 && port > UINT16_MAX) {
    errno = BadPort;
    return false;
  }

  // resolve the host name
  if (getaddrinfo(hostname, NULL, NULL, &hostinfo) != 0 || NULL == info)
    return false; // errno set by getaddrinfo

  for (cur = hostinfo; cur != NULL; cur = cur->ai_next) {
    if (AF_INET == cur->ai_family) {
      ((struct sockaddr_in *)cur->ai_addr)->sin_port = htons(port);
      break;
    }

    else if (AF_INET6 == cur->ai_family) {
      ((struct sockaddr_in6 *)cur->ai_addr)->sin6_port = htons(port);
      break;
    }
  }

  // copy the addrinfo for the host to the provided info structure
  if (NULL != cur)
    memcpy(info, cur, sizeof(*cur));

  // free the host addrinfo
  freeaddrinfo(hostinfo);

  return true;
}

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

bool socket_start(ctorm_app_t *app, const char *host) {
  struct addrinfo info;
  socklen_t       addrlen = 0;
  int             sockfd = -1, flag = 1;
  connection_t   *con = NULL;
  bool            ret = false;

  // parse the host to get the addrinfo structure
  if (!socket_parse_host(host, &info)) {
    debug("failed to parse the host: %s", ctorm_geterror());
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
    if (con == NULL && (con = connection_new()) == NULL) {
      debug("failed to create a new connection: %s", ctorm_geterror());
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

    if (!socket_set_opts(app, con->socket)) {
      error("failed to setsockopt for connection (con: %p, socket %d): %s", con, con->socket, strerror(errno));
      goto end;
    }

    debug("creating a thread for connection (con: %p, socket %d)", con, con->socket);
    pool_add(app->pool, (void *)connection_handle, (void *)con);

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
    connection_free(con);
  }

  return ret;
}
