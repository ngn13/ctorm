#include "conn.h"
#include "error.h"

#include "http.h"
#include "log.h"
#include "app.h"
#include "req.h"

#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <time.h>

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

ctorm_conn_t *ctorm_conn_new() {
  ctorm_conn_t *con = calloc(1, sizeof(ctorm_conn_t));

  if (NULL == con) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return NULL;
  }

  return con;
}

void ctorm_conn_free(ctorm_conn_t *con) {
  if (con->socket != 0)
    close(con->socket);
  free(con);
}

void ctorm_conn_handle(ctorm_conn_t *con) {
  conn_debug("handling new connection");

  bool            ret = false, persist = false, calc = false;
  ctorm_app_t    *app = con->app; // just for easy access
  struct timespec start, end;

  // define the HTTP request and the response
  ctorm_req_t req;
  ctorm_res_t res;

  do {
    // initialize the HTTP request and the response
    ctorm_req_init(&req, con);
    ctorm_res_init(&res, con);

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
  ctorm_conn_free(con);
}
