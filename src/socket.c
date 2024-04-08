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
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

pool_t *pool;
void handle_con(int *sa) {
  int s = *(int *)sa;
  debug("(Socket %d) Created thread", s);

  req_t req;
  req_init(&req);

  res_t res;
  res_init(&res);

  // measure the time that takes
  // to complete the request
  clock_t t = clock();
  bool badreq = false;
  handle_st ret = handle_request(&req, s);
  debug("Parsed request");

  char *buf;
  int bufsz;

  if (H_BADREQ == ret) {
    badreq = true;
    res.code = 400;
    goto CLOSE;
  }

  if (H_CONFAIL == ret) {
    res_free(&res);
    req_free(&req);
    free(sa);
    close(s);
    return;
  }

  if (DEBUG) {
    buf = malloc(req_size(&req));
    req_tostr(&req, buf);
    debug("\n%s", buf);
    free(buf);
  }

  res_set_version(&res, req.version);
  app_route(&req, &res);

CLOSE:
  bufsz = res_size(&res);
  buf = malloc(bufsz);

  res_tostr(&res, buf);
  send(s, buf, bufsz, 0);

  // finish the measurement and print out
  // the result
  if (!badreq) {
    t = (clock() - t) * 1000000;
    double passed = (((double)t) / CLOCKS_PER_SEC);
    log_req(passed, &req, &res);
  }

  req_free(&req);
  res_free(&res);

  free(buf);
  free(sa);
  close(s);
}

bool set_opts(int socket) {
  int flag = 1;
  t_socketop options[] = {
      // TCP delayed acknowledgment, buffers and combines multiple ACKs to
      // reduce overhead
      // it may delay the ACK response by up to 500ms, and we don't want that
      // because
      // slow bad fast good
      {IPPROTO_TCP, TCP_QUICKACK},

      // nagle's algorithm buffers and combines outgoing packets to solve the
      // "small-packet problem",
      // we want to send all the packets as fast as possible so we can disable
      // buffering with TCP_NODELAY
      {IPPROTO_TCP, TCP_NODELAY},
  };

  for (int i = 0; i < sizeof(options) / sizeof(t_socketop); i++)
    if (setsockopt(socket, options[i][0], options[i][1], &flag, sizeof(flag)) <
        0)
      return false;

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
             sizeof(timeout));
  fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) & (~O_NONBLOCK));
  return true;
}

void socket_con(struct evconnlistener *listener, evutil_socket_t fd,
                struct sockaddr *address, int socklen, void *ctx) {

  set_opts(fd);
  int *sock = malloc(sizeof(int));
  memcpy(sock, &fd, sizeof(int));
  pool_add(pool, (void *)handle_con, (void *)sock);
}

void socket_err(struct evconnlistener *listener, void *ctx) {
  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  error("Listener error: %s", evutil_socket_error_to_string(err));
  event_base_loopexit(base, NULL);
}

bool start_socket(app_t *app, char *addr, int port) {
  struct sockaddr_in address;
  bzero(&address, sizeof(address));
  pool = pool_init(THREADS);

  address.sin_family = AF_INET;
  inet_pton(AF_INET, addr, &(address.sin_addr));
  address.sin_port = htons(port);
  bool ret = false;

  struct event_base *base = event_base_new();
  if (NULL == base) {
    errno = EventFailed;
    return ret;
  }

  struct evconnlistener *listener = evconnlistener_new_bind(
      base, socket_con, NULL,
      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE, -1,
      (struct sockaddr *)&address, sizeof(address));
  if (NULL == listener) {
    errno = ListenFailed;
    return ret;
  }

  evconnlistener_get_fd(listener);
  evconnlistener_set_error_cb(listener, socket_err);

  event_base_dispatch(base);
  pool_stop(pool);

  errno = UnknownErr;
  return ret;
}
