#include "connection.h"
#include "errors.h"

#include "log.h"
#include "app.h"
#include "req.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

connection_t *connection_new() {
  connection_t *con = malloc(sizeof(connection_t));

  if (NULL == con) {
    errno = AllocFailed;
    return NULL;
  }

  bzero(con, sizeof(connection_t));
  return con;
}

void connection_free(connection_t *con) {
  if (con->socket != 0)
    close(con->socket);
  free(con);
}

#define __connection_lock(c)                                                                                           \
  if (((ctorm_app_t *)c->app)->config->lock_request)                                                                   \
  pthread_mutex_lock(&((ctorm_app_t *)c->app)->request_mutex)
#define __connection_unlock(c)                                                                                         \
  if (((ctorm_app_t *)c->app)->config->lock_request)                                                                   \
  pthread_mutex_unlock(&((ctorm_app_t *)c->app)->request_mutex)
#define __connection_debug(f, ...) debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET ") " f, con->socket, ##__VA_ARGS__)

void connection_handle(connection_t *con) {
  __connection_debug("handling new connection");

  ctorm_app_t *app          = con->app;
  clock_t      process_time = 0;
  bool         ret          = false;

  ctorm_req_t req;
  ctorm_res_t res;

  ctorm_req_init(&req, con);
  ctorm_res_init(&res, con);

  // if request logging is enabled measure the time that takes to complete the request
  if (!app->config->disable_logging)
    process_time = clock();

  if (!(ret = ctorm_req_start(&req))) {
    __connection_debug("received a bad request");
    res.code = 400;
    goto done;
  }

  __connection_debug("received a valid request");
  res.version = req.version;

  if (!app->config->server_header)
    ctorm_res_del(&res, "Server");

  // actually call the routes/middlewares
  __connection_lock(con);
  ctorm_app_route(app, &req, &res);
  __connection_unlock(con);
  ctorm_req_end(&req);

done:
  if (!ctorm_req_is_valid(&req))
    goto free; // no need to waste time tryna send response to a non-existent request

  __connection_debug("sending response (%d)", res.code);

  // send the complete response
  if (!ctorm_res_end(&res)) {
    __connection_debug("failed to send the response: %s", ctorm_geterror());
    goto free;
  }

  // finish the measurement and print out the result
  if (!app->config->disable_logging && ret) {
    process_time = (uint64_t)(clock() - process_time) / (uint64_t)(CLOCKS_PER_SEC / 1000000);
    __connection_lock(con);
    log(&req, &res, process_time);
    __connection_unlock(con);
  }

free:
  __connection_debug("closing connection");
  ctorm_req_free(&req);
  ctorm_res_free(&res);
  connection_free(con);
}
