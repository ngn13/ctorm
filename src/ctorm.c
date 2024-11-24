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

#include "../include/errors.h"
#include "../include/socket.h"

#include "../include/ctorm.h"
#include "../include/pool.h"

#include "../include/log.h"
#include "../include/req.h"
#include "../include/res.h"

#include <pthread.h>
#include <stdbool.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

app_t *signal_app;

void app_signal(int sig) {
  if (NULL == signal_app)
    return;

  signal_app->running = false;
}

app_t *app_new(app_config_t *_config) {
  app_t        *app    = malloc(sizeof(app_t));
  app_config_t *config = _config;

  bzero(app, sizeof(app_t));

  if (NULL == config) {
    config                 = malloc(sizeof(app_config_t));
    app->is_default_config = true;
    app_config_new(config);
  }

  app->config   = config;
  app->allroute = app_404;
  app->running  = false;

  if (config->tcp_timeout < 0) {
    errno = BadTcpTimeout;
    goto fail;
  } else if (config->tcp_timeout == 0)
    warn("Setting the TCP timeout to 0 may allow attackers to DoS your application");

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

  http_static_load();
  setbuf(stdout, NULL);
  return app;

fail:
  app_free(app);
  return NULL;
}

void app_free(app_t *app) {
  if (NULL == app)
    return;

  if (NULL != app->pool)
    pool_stop(app->pool);

  // reset setbuf
  routemap_t *cur = NULL, *prev = NULL;
  int         stdout_cp = dup(1);
  close(1);
  dup2(stdout_cp, 1);

  cur = app->middleware_maps;
  while (cur != NULL) {
    prev = cur;
    cur  = cur->next;
    free(prev);
  }

  cur = app->route_maps;
  while (cur != NULL) {
    prev = cur;
    cur  = cur->next;
    free(prev);
  }

  if (app->is_default_config)
    free(app->config);

  free(app);
}

void app_config_new(app_config_t *config) {
  if (NULL == config)
    return;

  config->max_connections = 1000;
  config->disable_logging = false;
  config->handle_signal   = true;
  config->server_header   = true;
  config->tcp_timeout     = 10;
  config->pool_size       = 30;
}

bool app_run(app_t *app, const char *addr) {
  if (NULL == app) {
    errno = InvalidAppPointer;
    return false;
  }

  if (NULL == addr) {
    errno = BadAddress;
    return false;
  }

  char  *save, *ip = NULL, *ports = NULL;
  size_t addrsize = strlen(addr) + 1;
  char   addrcpy[addrsize];
  bool   ret  = false;
  int    port = -1;

  memcpy(addrcpy, addr, addrsize);

  if (NULL == (ip = strtok_r(addrcpy, ":", &save))) {
    errno = BadAddress;
    return ret;
  }

  if (NULL == (ports = strtok_r(NULL, ":", &save))) {
    errno = BadAddress;
    return ret;
  }

  port = atoi(ports);

  if (port > UINT16_MAX || port <= 0) {
    errno = BadPort;
    return ret;
  }

  info("Starting the application on %s:%d", ip, port);

  app->running = true;
  signal_app   = app;

  if (app->config->handle_signal) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = app_signal;
    sa.sa_flags   = 0;
    sigaction(SIGINT, &sa, NULL);
  }

  if (socket_start(app, ip, port))
    ret = true;

  app->running = false;
  signal_app   = NULL;

  return ret;
}

bool app_static(app_t *app, char *path, char *dir) {
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

void app_all(app_t *app, route_t handler) {
  app->allroute = handler;
}

bool app_add(app_t *app, char *method, bool is_middleware, char *path, route_t handler) {
  if (NULL == app) {
    errno = InvalidAppPointer;
    return false;
  }

  if (path[0] != '/') {
    errno = BadPath;
    return false;
  }

  routemap_t *new = NULL, *cur = NULL, **maps = is_middleware ? &app->middleware_maps : &app->route_maps;

  if ((new = malloc(sizeof(routemap_t))) == NULL) {
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

void app_404(req_t *req, res_t *res) {
  res_set(res, "Content-Type", "text");
  res_send(res, "Not Found", 0);
  res->code = 404;
}

void app_route(app_t *app, req_t *req, res_t *res) {
  routemap_t *cur           = NULL;
  bool        found_handler = false;

  // call the middlewares, stop if a middleware cancels the request
  for (cur = app->middleware_maps; !req->cancel && cur != NULL; cur = cur->next) {
    if (cur->method != req->method && !cur->is_all)
      continue;

    if (!path_matches(cur->path, req->path))
      continue;

    cur->handler(req, res);
    found_handler = true;
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

  if (!res_sendfile(res, fp)) {
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
