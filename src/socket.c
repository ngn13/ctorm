#include "../include/socket.h"
#include "../include/errors.h"

#include "../include/options.h"

#include "../include/parse.h"
#include "../include/pool.h"

#include "../include/log.h"
#include "../include/req.h"
#include "../include/res.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include <pthread.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#include <time.h>

pthread_mutex_t socket_mutex;
req_t           req;
res_t           res;

void socket_handle(socket_args_t *_args) {
  socket_args_t *args = (socket_args_t *)_args;
  debug("(Socket %d) Created thread", args->socket);

  struct sockaddr *addr   = &args->addr;
  int              socket = args->socket;
  app_t           *app    = args->app;
  size_t           buffer_size;
  parse_ret_t      ret  = 0;
  clock_t          time = 0;

  pthread_mutex_lock(&socket_mutex);

  req_init(&req);
  res_init(&res);

  // set the request address
  switch (addr->sa_family) {
  case AF_INET:
    inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, req.addr, INET_ADDRSTRLEN);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6, &((struct sockaddr_in *)addr)->sin_addr, req.addr, INET6_ADDRSTRLEN);
    break;
  }

  /*

   * if request logging is enabled
   * measure the time that takes to complete the request

  */
  if (!app->config->disable_logging)
    time = clock();

  switch ((ret = parse_request(&req, socket))) {
  case RET_BADREQ:
    debug("(Socket %d) Received a bad request", args->socket);
    res.code = 400;
    goto close;

  case RET_TOOLARGE:
    debug("(Socket %d) Received a request that is too large", args->socket);
    res.code = 413;
    goto close;

  case RET_CONFAIL:
    debug("(Socket %d) Connection failed", args->socket);

    res_free(&res);
    req_free(&req);

    pthread_mutex_unlock(&socket_mutex);

    close(socket);
    free(args);

    return;

  default:
    debug("(Socket %d) Received a valid request", args->socket);
    break;
  }

  if (CTORM_DEBUG) {
    buffer_size = req_size(&req);
    char buffer[buffer_size];
    req_tostr(&req, buffer);
    debug("(Socket %d)\n%s", socket, buffer);
  }

  res.version = req.version;

  if (!app->config->server_header)
    res_del(&res, "Server");

  app_route(app, &req, &res);

close:
  buffer_size = res_size(&res);
  char buffer[buffer_size];

  res_tostr(&res, buffer);
  send(socket, buffer, buffer_size, 0);

  // finish the measurement and print out the result
  if (!app->config->disable_logging && RET_OK == ret) {
    time = (clock() - time) * 1000000;
    log_req((((double)time) / CLOCKS_PER_SEC), &req, &res);
  }

  req_free(&req);
  res_free(&res);

  pthread_mutex_unlock(&socket_mutex);

  free(args);
  close(socket);
}

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
  int              sockfd = -1, clientfd = -1, flag = 1;
  struct addrinfo *info = NULL, *cur = NULL;
  struct sockaddr  saddr, caddr;
  socklen_t        caddr_len = sizeof(caddr);
  socket_args_t   *args      = NULL;
  bool             ret       = false;

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

  pthread_mutex_init(&socket_mutex, NULL);

  while (app->running && (clientfd = accept(sockfd, &caddr, &caddr_len)) > 0) {
    debug("Accepted a new connection (socket %d)", clientfd);

    if (!socket_set_opts(app, clientfd)) {
      error("Failed to setsockopt for %d: %s", clientfd, strerror(errno));
      break;
    }

    args = malloc(sizeof(socket_args_t));
    memcpy(&args->addr, &caddr, sizeof(caddr));
    args->socket = clientfd;
    args->app    = app;

    pool_add(app->pool, (void *)socket_handle, (void *)args);
  }

  pthread_mutex_destroy(&socket_mutex);
  ret = true;

end:
  if (NULL != info)
    freeaddrinfo(info);

  if (sockfd > 0)
    close(sockfd);

  free(args);
  return ret;
}
