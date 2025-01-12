/*

 * ctorm | Simple web framework for C
 * Written by ngn (https://ngn.tf) (2024)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "errors.h"
#include "socket.h"

#include "pool.h"
#include "util.h"

#include "app.h"
#include "log.h"

#include <pthread.h>
#include <stdbool.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

ctorm_app_t *signal_app;

void __ctorm_signal_handler(int sig) {
  if (NULL == signal_app)
    return;

  debug("signal handler got called, stopping the app");
  signal_app->running = false;
}

void __ctorm_default_handler(ctorm_req_t *req, ctorm_res_t *res) {
  ctorm_res_set(res, "content-type", "text");
  ctorm_res_send(res, "not found", 0);
  res->code = 404;
}

ctorm_app_t *ctorm_app_new(ctorm_config_t *_config) {
  ctorm_app_t    *app    = malloc(sizeof(ctorm_app_t));
  ctorm_config_t *config = _config;

  bzero(app, sizeof(ctorm_app_t));

  if (NULL == config) {
    config                 = malloc(sizeof(ctorm_config_t));
    app->is_default_config = true;
    ctorm_config_new(config);
  }

  app->config   = config;
  app->allroute = __ctorm_default_handler;
  app->running  = false;

  if (config->tcp_timeout < 0) {
    errno = BadTcpTimeout;
    goto fail;
  }

  else if (config->tcp_timeout == 0)
    warn("setting the TCP timeout to 0 may allow attackers to DoS your application");

  if (config->max_connections <= 0) {
    errno = BadMaxConnCount;
    goto fail;
  }

  if (config->pool_size <= 0) {
    errno = BadPoolSize;
    goto fail;
  }

  if (NULL == (app->pool = pool_init(config->pool_size))) {
    errno = PoolFailed;
    goto fail;
  }

  if (config->lock_request && pthread_mutex_init(&app->request_mutex, NULL) != 0) {
    errno = MutexFail;
    goto fail;
  }

  http_static_load();
  setbuf(stdout, NULL);
  return app;

fail:
  ctorm_app_free(app);
  return NULL;
}

void ctorm_app_free(ctorm_app_t *app) {
  if (NULL == app)
    return;

  // reset stdout buffer
  int stdout_cp = dup(1);
  close(1);
  dup2(stdout_cp, 1);

  // free the application pool
  if (NULL != app->pool) {
    pool_stop(app->pool);
    app->pool = NULL;
  }

  // destroy the request mutex
  if (app->config->lock_request)
    pthread_mutex_destroy(&app->request_mutex);

  // free the routes and middlewares
  ctorm_routemap_t *cur = NULL, *prev = NULL;

  for (cur = app->middleware_maps; cur != NULL;) {
    prev = cur;
    cur  = cur->next;
    free(prev);
  }

  for (cur = app->route_maps; cur != NULL;) {
    prev = cur;
    cur  = cur->next;
    free(prev);
  }

  app->middleware_maps = NULL;
  app->route_maps      = NULL;

  // free the configuration
  if (app->is_default_config) {
    free(app->config);
    app->config = NULL;
  }

  // free the application
  free(app);
}

bool ctorm_app_run(ctorm_app_t *app, const char *host) {
  if (NULL == app) {
    errno = InvalidAppPointer;
    return false;
  }

  if (NULL == host) {
    errno = BadHost;
    return false;
  }

  app->running = true;
  signal_app   = app;

  if (app->config->handle_signal) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = __ctorm_signal_handler;
    sa.sa_flags   = 0;
    sigaction(SIGINT, &sa, NULL);
  }

  bool ret     = socket_start(app, host);
  app->running = false;
  signal_app   = NULL;

  return ret;
}

bool ctorm_app_static(ctorm_app_t *app, char *path, char *dir) {
  if (NULL == app) {
    errno = InvalidAppPointer;
    return false;
  }

  if (path[0] != '/') {
    errno = BadPath;
    return false;
  }

  app->staticpath = path;
  app->staticdir  = dir;
  return true;
}

bool ctorm_app_add(ctorm_app_t *app, char *method, bool is_middleware, char *path, ctorm_route_t handler) {
  if (NULL == app) {
    errno = InvalidAppPointer;
    return false;
  }

  if (*path != '/') {
    errno = BadPath;
    return false;
  }

  ctorm_routemap_t *new = NULL, *cur = NULL, **maps = is_middleware ? &app->middleware_maps : &app->route_maps;

  if ((new = malloc(sizeof(ctorm_routemap_t))) == NULL) {
    errno = AllocFailed;
    return false;
  }

  new->method        = http_method_id(method);
  new->is_middleware = is_middleware;
  new->handler       = handler;
  new->next          = NULL;
  new->path          = path;

  if (-1 == new->method)
    new->is_all = true;
  else
    new->is_all = false;

  if (NULL == (cur = *maps)) {
    *maps = new;
    return true;
  }

  while (NULL != cur->next)
    cur = cur->next;

  cur->next = new;
  return true;
}

void ctorm_app_all(ctorm_app_t *app, ctorm_route_t handler) {
  app->allroute = handler;
}

void ctorm_app_route(ctorm_app_t *app, ctorm_req_t *req, ctorm_res_t *res) {
  ctorm_routemap_t *cur           = NULL;
  bool              found_handler = false;

  // call the middlewares, stop if a middleware cancels the request
  for (cur = app->middleware_maps; !req->cancel && cur != NULL; cur = cur->next) {
    if (cur->method != req->method && !cur->is_all)
      continue;

    if (!path_matches(cur->path, req->path))
      continue;

    cur->handler(req, res);
  }

  // if the request is not cancelled, call all the routes
  if (!req->cancel) {
    for (cur = app->route_maps; !req->cancel && cur != NULL; cur = cur->next) {
      if (cur->method != req->method && !cur->is_all)
        continue;

      if (!path_matches(cur->path, req->path))
        continue;

      cur->handler(req, res);
      found_handler = true;
    }
  }

  // is the request already handled?
  if (found_handler)
    return;

  // if not check if we have a static route configured
  if (NULL == app->staticpath || NULL == app->staticdir || METHOD_GET != req->method)
    return app->allroute(req, res);

  // if so, check if this request can be handled with the static route
  size_t path_len   = strlen(req->path);
  size_t static_len = strlen(app->staticpath);
  char   staticpath[static_len + 2], realpath[path_len + 1];

  memcpy(staticpath, app->staticpath, static_len + 1);
  memcpy(realpath, req->path, path_len + 1);

  if (staticpath[static_len - 1] != '/') {
    staticpath[static_len]     = '/';
    staticpath[static_len + 1] = 0;
    static_len++;
  }

  if (!startswith(realpath, staticpath))
    return app->allroute(req, res);

  path_len -= static_len;
  if (path_len < 1)
    goto end;

  memcpy(realpath, realpath + static_len, path_len + 1);

  if (realpath[path_len - 1] == '/')
    goto end;

  char *fp = join(app->staticdir, realpath);
  if (contains(fp, '\\') || strstr(fp, "..") != NULL) {
    free(fp);
    goto end;
  }

  if (!ctorm_res_sendfile(res, fp)) {
    res->code = 404;
    free(fp);
    goto end;
  }

  res->code = 200;
  free(fp);
  return;

end:
  /*

   * if the request can't be handled, call the all route
   * by default its the 404 route

  */
  return app->allroute(req, res);
}
