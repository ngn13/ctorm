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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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
  app->all_route = __ctorm_default_handler;
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

  bool ret     = ctorm_socket_start(app, host);
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

  app->static_path = path;
  app->static_dir  = dir;
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
  app->all_route = handler;
}

uint64_t __ctorm_path_count_names(char *path) {
  uint64_t count = 0;

  for(; *path != 0; path++)
    if(*path == '/' && *(path+1) != 0)
      count++;

  return count+1;
}

char *__ctorm_path_next_name(char *path) {
  for(;*path != 0; path++)
    if(*path == '/')
      break;
  return path;
}

#define __ctorm_path_is_name_end(name) (*(name) == '/' || *(name) == 0)

bool __ctorm_app_route_matches(ctorm_routemap_t *route, ctorm_req_t *req) {
  // check if the request method and route method matches
  if (!ctorm_routemap_is_all(route) && route->method != req->method)
    return false;

  // remove '/' from both names, and check if both are and index route
  char *route_pos = route->path, *req_pos = req->path;
  uint64_t count = 0;

  if(*route_pos == '/')
    route_pos++;

  if(*req_pos== '/')
    req_pos++;

  if(*route_pos == 0 && *req_pos == 0)
    return true;

  // check if both paths have same amount of names (path components)
  if(__ctorm_path_count_names(route_pos) != (count = __ctorm_path_count_names(req_pos)))
    return false;

  char *key = NULL, *value = NULL;
  bool ret = false;

  // now we need to invidiualy compare every name in the path
  for(; count > 0; count--, route_pos = __ctorm_path_next_name(route_pos)+1, req_pos = __ctorm_path_next_name(req_pos)+1){
    // if the name is '*' then it's a wildcard route
    if(*route_pos == '*' && __ctorm_path_is_name_end(route_pos+1))
      continue;

    // if the name starts with ':' then it's a param
    if(*route_pos == ':' && !__ctorm_path_is_name_end(route_pos+1)){
      // duplicate the param name and the value and add it to the request
      if(NULL == (key = strdup(++route_pos)) || NULL == (value = strdup(req_pos))){
        errno = AllocFailed;
        goto end;
      }

      *__ctorm_path_next_name(key) = 0;
      *__ctorm_path_next_name(value) = 0;

      ctorm_pair_add(&req->params, key, value);
      continue;
    }

    // compare the route name with the request name
    if(!cu_strcmp_until(route_pos, req_pos, '/'))
      goto end;
  }

  ret = true;

end:
  if(!ret){
    ctorm_pair_free(req->params);
    req->params = NULL;
  }

  return ret;
}

void ctorm_app_route(ctorm_app_t *app, ctorm_req_t *req, ctorm_res_t *res) {
  ctorm_routemap_t *cur = NULL;

  // call the middlewares, stop if a middleware cancels the request
  for (cur = app->middleware_maps; !req->cancel && cur != NULL; cur = cur->next)
    if (__ctorm_app_route_matches(cur, req))
      cur->handler(req, res);

  if (req->cancel)
    return;

  // if the request is not cancelled, call all the routes
  for (cur = app->route_maps; cur != NULL; cur = cur->next)
    if (__ctorm_app_route_matches(cur, req))
      return cur->handler(req, res);

  // if not check if we have a static route configured
  if (NULL == app->static_path || NULL == app->static_dir || METHOD_GET != req->method)
    return app->all_route(req, res);

  // if so, check if this request can be handled with the static route
  size_t path_len   = cu_strlen(req->path);
  size_t static_len = cu_strlen(app->static_path);
  char   staticpath[static_len + 2], realpath[path_len + 1];

  memcpy(staticpath, app->static_path, static_len + 1);
  memcpy(realpath, req->path, path_len + 1);

  if (staticpath[static_len - 1] != '/') {
    staticpath[static_len]     = '/';
    staticpath[static_len + 1] = 0;
    static_len++;
  }

  if (!cu_startswith(realpath, staticpath))
    return app->all_route(req, res);

  path_len -= static_len;
  if (path_len < 1)
    goto end;

  memcpy(realpath, realpath + static_len, path_len + 1);

  if (realpath[path_len - 1] == '/')
    goto end;

  char *fp = cu_join(app->static_dir, realpath);
  if (cu_contains(fp, '\\') || strstr(fp, "..") != NULL) {
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
  return app->all_route(req, res);
}
