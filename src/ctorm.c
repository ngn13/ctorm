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

#include "../include/ctorm.h"
#include "../include/errors.h"
#include "../include/log.h"
#include "../include/pool.h"
#include "../include/req.h"
#include "../include/res.h"
#include "../include/socket.h"

#include <errno.h>
#include <event2/event.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

app_t *signal_app;

void app_signal(int sig) {
  if (NULL == signal_app)
    return;

  event_base_loopbreak(signal_app->base);
  signal_app->running = false;
}

app_t *app_new(app_config_t *_config) {
  app_t        *app    = malloc(sizeof(app_t));
  app_config_t *config = _config;

  if (NULL == config) {
    config                 = malloc(sizeof(app_config_t));
    app->is_default_config = true;
    app_config_new(config);
  }

  if (config->tcp_timeout < 0) {
    errno = BadTcpTimeout;
    goto fail;
  } else if (config->tcp_timeout == 0)
    warn("Setting the TCP timeout to 0 may allow attackers to DoS your application");

  if (config->pool_size <= 0) {
    errno = BadPoolSize;
    goto fail;
  }

  if (NULL == (app->base = event_base_new())) {
    errno = EventFailed;
    goto fail;
  }

  if (NULL == (app->pool = pool_init(config->pool_size))) {
    errno = PoolFailed;
    goto fail;
  }

  http_static_load();

  app->config     = config;
  app->allroute   = app_404;
  app->staticdir  = NULL;
  app->staticpath = NULL;
  app->maps       = NULL;
  app->running    = false;

  setbuf(stdout, NULL);
  return app;

fail:
  free(app);
  return NULL;
}

void app_free(app_t *app) {
  if (NULL == app)
    return;

  event_base_free(app->base);
  pool_stop(app->pool);

  // reset setbuf
  int stdout_cp = dup(1);
  close(1);
  dup2(stdout_cp, 1);

  routemap_t *cur = app->maps, *prev = NULL;
  while (NULL != cur) {
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

  ip = strtok_r(addrcpy, ":", &save);
  if (NULL == ip) {
    errno = BadAddress;
    return ret;
  }

  ports = strtok_r(NULL, ":", &save);
  if (NULL == ports) {
    errno = BadAddress;
    return ret;
  }

  port = atoi(ports);
  if (port > 65535 || port <= 0) {
    errno = BadPort;
    return ret;
  }

  info("Starting the application on %s:%d", ip, port);

  app->running = true;
  signal_app   = app;

  if (app->config->handle_signal)
    signal(SIGINT, app_signal);

  if (socket_start(app, ip, port))
    ret = true;

  if (app->config->handle_signal)
    signal(SIGINT, SIG_DFL);

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

  routemap_t *new = malloc(sizeof(routemap_t));
  if (NULL == new) {
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

  if (NULL == app->maps) {
    app->maps = new;
    return true;
  }

  routemap_t *cur = app->maps;
  while (NULL != cur) {
    if (NULL == cur->next) {
      cur->next = new;
      return true;
    }
    cur = cur->next;
  }

  return false;
}

void app_404(req_t *req, res_t *res) {
  res_set(res, "Content-Type", "text");
  res_send(res, "Not Found", 0);
  res->code = 404;
}

void app_route(app_t *app, req_t *req, res_t *res) {
  // matches will be stored in a list,
  // this way we can make sure that we call
  // middlewares before the actual routes
  routemap_t **middlewares = malloc(sizeof(routemap_t *));
  size_t       mindex      = 0;

  routemap_t **routes = malloc(sizeof(routemap_t *));
  size_t       rindex = 0;

  middlewares[mindex] = NULL;
  routes[rindex]      = NULL;

  routemap_t *cur = app->maps;
  while (NULL != cur) {
    // if the method does not match and the route does not handle
    // all the methods, continue
    if (cur->method != req->method && !cur->is_all)
      goto cont;

    // compare the paths
    if (!path_matches(cur->path, req->path))
      goto cont;

    // add it to the middleware list
    if (cur->is_middleware) {
      middlewares[mindex++] = cur;
      middlewares           = realloc(middlewares, sizeof(routemap_t *) * (mindex + 1));
      middlewares[mindex++] = NULL;
      goto cont;
    }

    // add it to the routes list
    routes[rindex++] = cur;
    routes           = realloc(routes, sizeof(routemap_t *) * (rindex + 1));
    routes[rindex++] = NULL;

  cont:
    cur = cur->next;
  }

  // don't call the middleware if there's no route handler
  if (mindex > 0 || rindex == 0)
    mindex = 0;

  // call the middlewares, stop if a middleware cancels the request
  for (int i = 0; !req->cancel && middlewares[i] != NULL; i++)
    middlewares[i]->handler(req, res);

  // if the request is not cancelled, call all the routes
  if (!req->cancel) {
    for (int i = 0; routes[i] != NULL; i++)
      routes[i]->handler(req, res);
  }

  // cleanup the lists
  free(middlewares);
  free(routes);

  // is the request already handled?
  if (rindex != 0 || mindex != 0)
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
  // if the request can't be handled, call the all route
  // by default its the 404 route
  return app->allroute(req, res);
}
