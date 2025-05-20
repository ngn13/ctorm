#define _GNU_SOURCE
#define __USE_GNU

#include "error.h"
#include "conn.h"
#include "pool.h"
#include "log.h"

#include <sys/socket.h>
#include <sys/time.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <time.h>

// TODO: maybe move these to socket.c and remove conn.c entirely?

struct ctorm_conn {
  ctorm_app_t    *app;
  int             socket;
  struct sockaddr addr;
};

// request thread lock macro
#define conn_lock(c)                                                           \
  if (((ctorm_app_t *)c->app)->config->lock_request)                           \
  pthread_mutex_lock(&((ctorm_app_t *)c->app)->req_mutex)

// request thread unlock macro
#define conn_unlock(c)                                                         \
  if (((ctorm_app_t *)c->app)->config->lock_request)                           \
  pthread_mutex_unlock(&((ctorm_app_t *)c->app)->req_mutex)

// a neat debug macro
#define conn_debug(f, ...)                                                     \
  debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET ") " f,                    \
      con->socket,                                                             \
      ##__VA_ARGS__)

void _ctorm_conn_free(struct ctorm_conn *con) {
  if (con->socket != 0)
    close(con->socket);
  free(con);
}

void _ctorm_conn_handle(void *data) {
  struct ctorm_conn *con = data;
  ctorm_app_t       *app = con->app;

  conn_debug("handling new connection");

  bool            ret = false, persist = false, calc = false;
  struct timespec start, end;

  // define the HTTP request and the response
  ctorm_req_t req;
  ctorm_res_t res;

  do {
    // initialize the HTTP request and the response
    ctorm_req_init(&req, con->socket, &con->addr);
    ctorm_res_init(&res, con->socket, &con->addr);

    // if disabled, remove the server header from the response
    if (!app->config->server_header)
      ctorm_res_del(&res, CTORM_HTTP_SERVER);

    // if not disabled, save the current time to calculate the process time
    if (!app->config->disable_logging)
      calc = clock_gettime(CLOCK_REALTIME, &start) == 0;

    // receive the HTTP request
    ret         = ctorm_req_recv(&req);
    res.version = req.version;
    res.code    = req.code;

    // route the request if we successfuly received a HTTP request
    if (ret) {
      /*

       * we lock the request mutex of the app before actually routing it, this
       * means only one request can be routed in a given time, however this lock
       * can be disabled with the lock_request configuration option

      */
      conn_lock(con);
      ctorm_app_route(app, &req, &res);
      conn_unlock(con);
    }

    // debug print if we failed to receive a HTTP request
    else
      conn_debug("received an invalid HTTP request");

    persist = ctorm_req_persist(&req);
    conn_debug("sending a %d response", res.code);

    // send the complete response
    if (!ctorm_res_send(&res)) {
      conn_debug("failed to send the response: %s", ctorm_error());
      goto next;
    }

    // finish process time measurement and log the request
    if (ret && !app->config->disable_logging && calc &&
        clock_gettime(CLOCK_REALTIME, &end) == 0) {
      struct timeval end_val, start_val;

      TIMESPEC_TO_TIMEVAL(&start_val, &start);
      TIMESPEC_TO_TIMEVAL(&end_val, &end);

      conn_lock(con);
      log(&req, &res, end_val.tv_usec - start_val.tv_usec);
      conn_unlock(con);
    }

  next:
    // reset the request and response data
    ctorm_req_free(&req);
    ctorm_res_free(&res);
  } while (persist);

  // close & free the connection
  conn_debug("closing connection");
  _ctorm_conn_free(con);
}

void _ctorm_conn_kill(void *data) {
  struct ctorm_conn *con = data;
  shutdown(con->socket, SHUT_RDWR);
}

bool ctorm_conn_new(ctorm_app_t *app, struct sockaddr *addr, int socket) {
  struct ctorm_conn *con = calloc(1, sizeof(struct ctorm_conn));

  if (NULL == con) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return false;
  }

  con->app    = app;
  con->socket = socket;
  memcpy(&con->addr, addr, sizeof(con->addr));

  // TODO: check if we have too much work, if so wait for one to finish
  if (!ctorm_pool_add(app->pool, _ctorm_conn_handle, _ctorm_conn_kill, con)) {
    free(con);
    return false; // errno set by ctorm_pool_add()
  }

  return true;
}
