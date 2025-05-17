/*

 * ctorm | Simple web framework for C
 * Written by ngn (https://ngn.tf) (2025)

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

#include "error.h"
#include "http.h"
#include "pair.h"
#include "req.h"
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
#include <unistd.h>
#include <stdio.h>

ctorm_app_t    *_ctorm_signal_head  = NULL;
pthread_mutex_t _ctorm_signal_mutex = PTHREAD_MUTEX_INITIALIZER;

#define app_check_ptr(ret)                                                     \
  do {                                                                         \
    if (NULL == app) {                                                         \
      errno = CTORM_ERR_BAD_APP_PTR;                                           \
      return ret;                                                              \
    }                                                                          \
  } while (0)

void _app_signal_handler(int sig) {
  cu_unused(sig);

  if (NULL == _ctorm_signal_head)
    return;

  pthread_t    self = pthread_self();
  ctorm_app_t *cur  = _ctorm_signal_head;

  /*

   * we'll loop through the app list, lock it to make sure it's not modified by
   * another thread while we loop through it

  */
  pthread_mutex_lock(&_ctorm_signal_mutex);

  // stop all the apps in the current thread
  for (; cur != NULL; cur = cur->next) {
    if (self != cur->thread)
      continue;

    debug("stopping the app %p", cur);
    cur->running = false;
  }

  // loop is complete, unlock the app list
  pthread_mutex_unlock(&_ctorm_signal_mutex);
}

void __ctorm_default_handler(ctorm_req_t *req, ctorm_res_t *res) {
  cu_unused(req);
  ctorm_res_set(res, "content-type", "text");
  ctorm_res_body(res, "not found", 0);
  ctorm_res_code(res, 404);
}

ctorm_app_t *ctorm_app_new(ctorm_config_t *config) {
  ctorm_app_t *app = malloc(sizeof(ctorm_app_t));

  if (NULL == app) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return NULL;
  }

  // clear the app structure
  bzero(app, sizeof(ctorm_app_t));

  if (NULL == config) {
    if (NULL == (config = ctorm_config_new(NULL)))
      goto fail; // errno set by ctorm_config_new()

    app->is_default_config = true;
    ctorm_config_new(config);
  }

  else if (!ctorm_config_check(config))
    goto fail;

  app->default_route = __ctorm_default_handler;
  app->config        = config;
  app->running       = false;

  if (NULL == (app->pool = ctorm_pool_new(config->pool_size))) {
    errno = CTORM_ERR_POOL_FAIL;
    goto fail;
  }

  if ((config->lock_request &&
          pthread_mutex_init(&app->req_mutex, NULL) != 0) ||
      pthread_mutex_init(&app->mod_mutex, NULL) != 0) {
    errno = CTORM_ERR_MUTEX_FAIL;
    goto fail;
  }

  ctorm_http_load();
  return app;

fail:
  ctorm_app_free(app);
  return NULL;
}

void ctorm_app_free(ctorm_app_t *app) {
  if (NULL == app)
    return;

  // free the server's thread pool
  if (NULL != app->pool) {
    ctorm_pool_free(app->pool);
    app->pool = NULL;
  }

  // free the routes
  struct ctorm_routemap *prev = NULL;

  while (app->routes != NULL) {
    prev        = app->routes;
    app->routes = app->routes->next;
    free(prev);
  }

  pthread_mutex_destroy(&app->mod_mutex);

  if (NULL != app->config) {
    // destroy the request mutex
    if (app->config->lock_request)
      pthread_mutex_destroy(&app->req_mutex);

    // free the configuration if it's default
    if (app->is_default_config)
      free(app->config);
    app->config = NULL;
  }

  // free the application
  free(app);
}

bool ctorm_app_run(ctorm_app_t *app, const char *addr) {
  app_check_ptr(false);

  if (NULL == addr) {
    errno = CTORM_ERR_BAD_ADDR_PTR;
    return false;
  }

  struct sigaction sa;
  bool             ret = false;

  // if signal handling is enabled, add app to the signal list
  if (app->config->handle_signal) {
    if (NULL == _ctorm_signal_head) {
      _ctorm_signal_head = app;
      sigemptyset(&sa.sa_mask);
      sa.sa_handler = _app_signal_handler;
      sa.sa_flags   = 0;
      sigaction(SIGINT, &sa, NULL);
    }

    else {
      pthread_mutex_lock(&_ctorm_signal_mutex);
      app->next          = _ctorm_signal_head;
      _ctorm_signal_head = app;
      pthread_mutex_unlock(&_ctorm_signal_mutex);
    }
  }

  // save the current thread before starting the server
  app->thread = pthread_self();

  // start the web server and wait until it's done
  app->running = true;
  ret          = ctorm_socket_start(app, (char *)addr);
  app->running = false;

  // if signal handling is enabled, remove app from the signal list
  if (app->config->handle_signal) {
    ctorm_app_t *cur = _ctorm_signal_head, *prev = NULL;

    // find the app in the list
    for (; cur != NULL && cur != app; cur = cur->next)
      prev = cur;

    // remove the app from the list
    pthread_mutex_lock(&_ctorm_signal_mutex);
    if (NULL == prev)
      _ctorm_signal_head = app->next;
    else
      prev->next = app->next;
    pthread_mutex_unlock(&_ctorm_signal_mutex);

    // remove the signal handler if the list is empty
    if (NULL == _ctorm_signal_head) {
      sa.sa_handler = SIG_DFL;
      sa.sa_flags   = 0;
      sigaction(SIGINT, &sa, NULL);
    }
  }

  return ret;
}

bool ctorm_app_stop(ctorm_app_t *app) {
  app_check_ptr(false);

  if (app->config->handle_signal)
    pthread_kill(app->thread, SIGINT);
  else
    app->running = false;

  return true;
}

bool ctorm_app_local(ctorm_app_t *app, char *name, void *value) {
  return NULL != ctorm_pair_add(&app->locals, name, value);
}

bool ctorm_app_static(ctorm_app_t *app, char *path, char *dir) {
  app_check_ptr(false);

  if (*path != '/') {
    errno = CTORM_ERR_BAD_PATH;
    return false;
  }

  cu_str_set(&app->static_path, path);
  cu_str_set(&app->static_dir, dir);
  return true;
}

bool ctorm_app_add(ctorm_app_t *app, ctorm_http_method_t method, char *path,
    ctorm_route_t handler) {
  app_check_ptr(false);

  if (*path != '/') {
    errno = CTORM_ERR_BAD_PATH;
    return false;
  }

  struct ctorm_routemap *new = NULL, *cur = NULL;

  if ((new = malloc(sizeof(*new))) == NULL) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return false;
  }

  new->method  = method;
  new->handler = handler;
  new->next    = NULL;
  new->path    = path;

  if (NULL == (cur = app->routes)) {
    app->routes = new;
    return true;
  }

  while (NULL != cur->next)
    cur = cur->next;

  cur->next = new;
  return true;
}

void ctorm_app_all(ctorm_app_t *app, ctorm_route_t handler) {
  if (NULL != app && NULL != handler)
    app->default_route = handler;
}

uint64_t _ctorm_path_count_names(char *path) {
  uint64_t count = 0;

  for (; *path != 0; path++)
    if (*path == '/' && *(path + 1) != 0)
      count++;

  return count + 1;
}

char *_ctorm_path_next_name(char *path) {
  for (; *path != 0; path++)
    if (*path == '/')
      break;
  return path;
}

#define ctorm_path_is_name_end(name) (*(name) == '/' || *(name) == 0)

bool _ctorm_app_route_matches(struct ctorm_routemap *route, ctorm_req_t *req) {
  // check if the request method and route method matches
  if (!ctorm_routemap_is_all(route) && route->method != req->method)
    return false;

  // remove '/' from both names, and check if both are and index route
  char    *route_pos = route->path, *req_pos = req->path;
  uint64_t count = 0;

  if (*route_pos == '/')
    route_pos++;

  if (*req_pos == '/')
    req_pos++;

  if (*route_pos == 0 && *req_pos == 0)
    return true;

  // check if both paths have same amount of names (path components)
  if (_ctorm_path_count_names(route_pos) !=
      (count = _ctorm_path_count_names(req_pos)))
    return false;

  char *key = NULL, *value = NULL;
  bool  ret = false;

  /*

   * now we need to individually compare every name in the path

   * BUG: gh workflow uses clang-format 19.1.1 atm, which uses a different
   *      formatting for this loop for some reason, so temporarily disable the
   *      formatting here

  */
  // clang-format off
  for (; count > 0; count--,
      route_pos = _ctorm_path_next_name(route_pos) + 1,
      req_pos   = _ctorm_path_next_name(req_pos) + 1) {
    // clang-format on

    // if the name is '*' then it's a wildcard route
    if (*route_pos == '*' && ctorm_path_is_name_end(route_pos + 1))
      continue;

    // if the name starts with ':' then it's a URL parameter
    if (*route_pos == ':' && !ctorm_path_is_name_end(route_pos + 1)) {
      // duplicate the parameter name and the value and add it to the request
      if (NULL == (key = strdup(++route_pos)) ||
          NULL == (value = strdup(req_pos))) {
        errno = CTORM_ERR_ALLOC_FAIL;
        goto end;
      }

      *_ctorm_path_next_name(key)   = 0;
      *_ctorm_path_next_name(value) = 0;

      ctorm_pair_add(&req->params, key, value);
      continue;
    }

    // compare the route name with the request name
    if (!cu_strcmpu(route_pos, req_pos, '/'))
      goto end;
  }

  ret = true;

end:
  if (!ret) {
    ctorm_pair_free(req->params);
    req->params = NULL;
  }

  return ret;
}

void ctorm_app_route(ctorm_app_t *app, ctorm_req_t *req, ctorm_res_t *res) {
  struct ctorm_routemap *cur         = NULL;
  bool                   found_route = false;

  // copy the locals to the request
  ctorm_pair_next(app->locals, local)
      ctorm_req_local(req, local->key, local->value);

  // call the routes, stop if a route cancels the request
  for (cur = app->routes; !req->cancel && cur != NULL; cur = cur->next) {
    if (_ctorm_app_route_matches(cur, req)) {
      cur->handler(req, res);
      found_route = true;
    }
  }

  // if we found at least one matching route, then route is complete
  if (found_route)
    return;

  // if not check if we have a static route configured
  while (!cu_str_empty(&app->static_path) && !cu_str_empty(&app->static_dir) &&
         CTORM_HTTP_GET == req->method) {
    // if so, check if this request can be handled with the static route
    uint64_t path_len = cu_strlen(req->path), static_fp_len = 0, sub_len = 0;
    char    *path_ptr = req->path;

    // static request path will be longer than the static route path
    if (path_len <= (uint64_t)app->static_path.len)
      break;

    // get the position of the sub static directory path
    if ('/' == path_ptr[app->static_path.len - 1])
      sub_len = app->static_path.len;
    else if ('/' == path_ptr[app->static_path.len])
      sub_len = app->static_path.len + 1;
    else
      break;

    // make sure the sub directory is not empty
    if (*(path_ptr += sub_len) == 0)
      break;

    // compare the start of the path
    if (!cu_startswith(req->path, app->static_path.buf))
      break;

    // this is just to prevent a potential file disclosure attack
    if (cu_contains(path_ptr, '\\') || strstr(path_ptr, "..") != NULL)
      break;

    // join the static directory with the sub directory
    static_fp_len = app->static_dir.len + 1 + path_len + 1;
    char static_fp[static_fp_len];

    snprintf(static_fp, static_fp_len, "%s/%s", app->static_dir.buf, path_ptr);

    if (!ctorm_res_file(res, static_fp))
      break;

    res->code = 200;
    return;
  }

  /*

   * if the request can't be handled, call the all route which is the route that
   * handles all the unhandled routes

   * by default it's the 404 route

  */
  app->default_route(req, res);
}
