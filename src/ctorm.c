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
#include <regex.h>
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

app_t *app_new(app_config_t *config) {
  app_t *app = malloc(sizeof(app_t));

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
  event_base_free(app->base);
  pool_stop(app->pool);

  // reset setbuf
  int stdout_cp = dup(1);
  close(1);
  dup2(stdout_cp, 1);

  free(app);
}

void app_config_new(app_config_t *config) {
  config->disable_logging = false;
  config->handle_signal   = true;
  config->server_header   = true;
  config->tcp_timeout     = 10;
  config->pool_size       = 30;
}

bool app_run(app_t *app, const char *addr) {
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

void app_static(app_t *app, char *path, char *dir) {
  app->staticpath = path;
  app->staticdir  = dir;
}

void app_all(app_t *app, route_t handler) {
  app->allroute = handler;
}

bool app_add(app_t *app, char *method, bool is_regex, char *path, route_t handler) {
  routemap_t *new = malloc(sizeof(routemap_t));
  if (NULL == new) {
    errno = AllocFailed;
    return false;
  }

  new->method   = http_method_id(method);
  new->is_regex = is_regex;
  new->handler  = handler;
  new->path     = path;
  new->next     = NULL;

  if (NULL == app->maps) {
    app->maps = new;
    return true;
  }

  routemap_t *cur = app->maps;
  while (true) {
    if (NULL == cur->next) {
      cur->next = new;
      return true;
    }
    cur = app->maps->next;
  }
}

void app_404(req_t *req, res_t *res) {
  res_set(res, "Content-Type", "text");
  res_send(res, "Not Found", 0);
  res->code = 404;
}

void app_route(app_t *app, req_t *req, res_t *res) {
  bool    found = false;
  regex_t regex;

  routemap_t *cur = app->maps;
  while (NULL != cur) {
    if (cur->method != req->method) {
      cur = cur->next;
      continue;
    }

    if (cur->is_regex)
      goto regex;

    if (eq(cur->path, req->path)) {
      cur->handler(req, res);
      found = true;
    }

    cur = cur->next;
    continue;

  regex:
    if (regcomp(&regex, cur->path, 0)) {
      error("Skipping bad route: %s", cur->path);
      cur = cur->next;
      continue;
    }

    if (regexec(&regex, req->path, 0, NULL, 0) == REG_NOMATCH) {
      regfree(&regex);
      cur = cur->next;
      continue;
    }

    regfree(&regex);
    cur->handler(req, res);
    found = true;
    cur   = cur->next;
  }

  if (found)
    goto done;

  if (NULL == app->staticpath || NULL == app->staticdir || METHOD_GET != req->method)
    goto done;

  char *staticpath, *realpath = strdup(req->path);
  int   staticpath_len = strlen(app->staticpath);

  if (app->staticpath[staticpath_len - 1] != '/')
    staticpath = join(app->staticpath, "");
  else
    staticpath = strdup(app->staticpath);

  if (startswith(realpath, staticpath)) {
    memmove(realpath, realpath + strlen(staticpath), strlen(realpath));
    if (realpath[strlen(realpath) - 1] == '/')
      goto skip;

    char *fp = join(app->staticdir, realpath);
    if (contains(fp, '\\') || strstr(fp, "..") != NULL) {
      free(fp);
      goto skip;
    }

    if (!file_canread(fp)) {
      free(fp);
      goto skip;
    }

    res_sendfile(res, fp);
    res->code = 200;
    found     = true;
    free(fp);
  }

skip:
  free(staticpath);
  free(realpath);

done:
  if (!found)
    app->allroute(req, res);
}
