#include "../include/socket.h"
#include "../include/errors.h"
#include "../include/log.h"
#include "../include/parse.h"
#include "../include/pool.h"
#include "../include/req.h"
#include "../include/res.h"

#include <arpa/inet.h>
#include <errno.h>
#include <event2/listener.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

void socket_handle(socket_args_t *_args) {
  socket_args_t *args = (socket_args_t *)_args;
  debug("(Socket %d) Created thread", args->socket);

  struct sockaddr *addr   = args->address;
  evutil_socket_t  socket = args->socket;
  app_t           *app    = args->app;
  size_t           buffer_size;

  req_t req;
  req_init(&req);

  res_t res;
  res_init(&res);

  // set the request address
  if (addr->sa_family == AF_INET) {
    req.addr = malloc(INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, req.addr, INET_ADDRSTRLEN);
  } else if (addr->sa_family == AF_INET6) {
    req.addr = malloc(INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &((struct sockaddr_in *)addr)->sin_addr, req.addr, INET6_ADDRSTRLEN);
  }

  // measure the time that takes
  // to complete the request
  clock_t     time = clock();
  parse_ret_t ret  = parse_request(&req, socket);

  switch (ret) {
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

    close(socket);
    free(args);
    return;

  default:
    debug("(Socket %d) Received a valid request", args->socket);
    break;
  }

  if (DEBUG) {
    buffer_size = req_size(&req);
    char buffer[buffer_size];
    req_tostr(&req, buffer);
    debug("(Socket %d)\n%s", socket, buffer);
  }

  res.version = req.version;
  if (!app->config->server_header)
    res_del(&res, "Server");

  if (app->config->lock_request)
    pthread_mutex_lock(&app->request_mutex);

  app_route(app, &req, &res);

  if (app->config->lock_request)
    pthread_mutex_unlock(&app->request_mutex);

close:
  buffer_size = res_size(&res);
  char buffer[buffer_size];

  res_tostr(&res, buffer);
  send(socket, buffer, buffer_size, 0);

  // finish the measurement and print out
  // the result
  if (RET_OK == ret) {
    time          = (clock() - time) * 1000000;
    double passed = (((double)time) / CLOCKS_PER_SEC);
    if (!app->config->disable_logging)
      log_req(passed, &req, &res);
  }

  req_free(&req);
  res_free(&res);

  free(args);
  close(socket);
}

bool socket_set_opts(app_t *app, int socket) {
  struct timeval timeout;
  bzero(&timeout, sizeof(timeout));
  int flag = 1;

  // TCP delayed acknowledgment, buffers and combines multiple ACKs to reduce overhead
  // it may delay the ACK response by up to 500ms, and we don't want that because slow bad fast good
  if (setsockopt(socket, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0)
    return false;

  // nagle's algorithm buffers and combines outgoing packets to solve the "small-packet problem",
  // we want to send all the packets as fast as possible so we can disable buffering with TCP_NODELAY
  if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0)
    return false;

  // set the socket timeout
  if (app->config->tcp_timeout > 0) {
    timeout.tv_sec  = app->config->tcp_timeout;
    timeout.tv_usec = 0;

    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  }

  // make the socket blocking
  fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) & (~O_NONBLOCK));
  return true;
}

void socket_con(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
  socket_args_t *args = malloc(sizeof(socket_args_t));
  args->address       = address;
  args->socket        = fd;
  args->app           = ctx;

  if (!socket_set_opts(args->app, args->socket))
    error("Failed to setsockopt for %d: %s", socket, strerror(errno));
  pool_add(args->app->pool, (void *)socket_handle, (void *)args);
}

void socket_err(struct evconnlistener *listener, void *ctx) {
  struct event_base *base = evconnlistener_get_base(listener);
  int                err  = EVUTIL_SOCKET_ERROR();
  error("Listener error: %s", evutil_socket_error_to_string(err));
  event_base_loopexit(base, NULL);
}

bool socket_start(app_t *app, char *addr, int port) {
  struct sockaddr_in address;
  bool               ret = false;

  bzero(&address, sizeof(address));
  inet_pton(AF_INET, addr, &(address.sin_addr));
  address.sin_family = AF_INET;
  address.sin_port   = htons(port);

  struct evconnlistener *listener = evconnlistener_new_bind(app->base,
      socket_con,
      app,
      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE,
      -1,
      (struct sockaddr *)&address,
      sizeof(address));

  if (NULL == listener) {
    errno = ListenFailed;
    return ret;
  }

  evconnlistener_set_error_cb(listener, socket_err);
  event_base_dispatch(app->base);
  evconnlistener_free(listener);
  return true;
}
