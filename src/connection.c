#include "../include/connection.h"
#include "../include/options.h"
#include "../include/errors.h"

#include "../include/ctorm.h"
#include "../include/log.h"

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
  if (((app_t *)c->app)->config->lock_request)                                                                         \
  pthread_mutex_lock(&((app_t *)c->app)->request_mutex)
#define __connection_unlock(c)                                                                                         \
  if (((app_t *)c->app)->config->lock_request)                                                                         \
  pthread_mutex_unlock(&((app_t *)c->app)->request_mutex)
#define __connection_debug(f, ...) debug("(" FG_BOLD "Socket " FG_CYAN "%d" FG_RESET ") " f, con->socket, ##__VA_ARGS__)

void connection_handle(connection_t *con) {
  __connection_debug("Handling new connection");

  app_t  *app  = con->app;
  clock_t time = 0;
  bool    ret  = false;

  req_t req;
  res_t res;

  req_init(&req, con);
  res_init(&res, con);

  // if request logging is enabled measure the time that takes to complete the request
  if (!app->config->disable_logging)
    time = clock();

  if (!(ret = req_start(&req))) {
    __connection_debug("Received a bad request");
    res.code = 400;
    goto done;
  }

  __connection_debug("Received a valid request");
  res.version = req.version;

  if (!app->config->server_header)
    res_del(&res, "Server");

  // actually call the routes/middlewares
  __connection_lock(con);
  app_route(app, &req, &res);
  __connection_unlock(con);
  req_end(&req);

done:
  __connection_debug("Sending response (%d)", res.code);

  // send the complete response
  if (!res_end(&res)) {
    __connection_debug("Failed to send the response: %s", app_geterror());
    goto free;
  }

  // finish the measurement and print out the result
  if (!app->config->disable_logging && ret) {
    time = (clock() - time) * 1000000;
    __connection_lock(con);
    log_req((((double)time) / CLOCKS_PER_SEC), &req, &res);
    __connection_unlock(con);
  }

free:
  __connection_debug("Closing connection");
  req_free(&req);
  res_free(&res);
  connection_free(con);
}
