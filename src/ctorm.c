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
#include "../include/req.h"
#include "../include/res.h"
#include "../include/socket.h"

#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

app_t *app_new() {
  app_t *app = malloc(sizeof(app_t));
  app->allroute = app_404;
  app->staticdir = NULL;
  app->staticpath = NULL;
  app->maps = NULL;
  setbuf(stdout, NULL);
  return app;
}

bool app_run(app_t *app, const char *addr) {
  char *save, *ip = NULL, *ports = NULL;
  char *addrcpy = strdup(addr);
  int port = -1;

  ip = strtok_r(addrcpy, ":", &save);
  if (NULL == ip) {
    errno = BadAddress;
    goto FAIL;
  }

  ports = strtok_r(NULL, ":", &save);
  if (NULL == ports) {
    errno = BadAddress;
    goto FAIL;
  }

  port = atoi(ports);
  if (port > 65535 || port <= 0) {
    errno = BadPort;
    goto FAIL;
  }

  info("Starting the application on http://%s:%d", ip, port);
  if (socket_start(app, ip, port)) {
    free(addrcpy);
    return true;
  }

FAIL:
  free(addrcpy);
  return false;
}

void app_static(app_t *app, char *path, char *dir) {
  app->staticpath = path;
  app->staticdir  = dir;
}

void app_all(app_t *app, route_t handler) { app->allroute = handler; }

bool app_add(app_t *app, char *method, bool regex, char *path, route_t handler) {
  routemap_t *new = malloc(sizeof(routemap_t));
  if (NULL == new) {
    errno = AllocFailed;
    return false;
  }

  new->method = http_methodid(method);
  new->handler = handler;
  new->regex = regex;
  new->path = path;
  new->next = NULL;

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
  res_send(res, "Not Found");
  res->code = 404;
}

void app_route(app_t *app, req_t *req, res_t *res) {
  bool found = false;
  regex_t regex;

  routemap_t *cur = app->maps;
  while (NULL != cur) {
    if (cur->method != req->method) {
      cur = cur->next;
      continue;
    }

    if (cur->regex)
      goto REGEX;

    if (eq(cur->path, req->path)) {
      cur->handler(req, res);
      found = true;
    }

    cur = cur->next;
    continue;

  REGEX:
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
    cur = cur->next;
  }

  if (found)
    goto DONE;

  if (NULL == app->staticpath || NULL == app->staticdir || METHOD_GET != req->method)
    goto DONE;

  char *staticpath, *realpath = strdup(req->path);
  int staticpath_len = strlen(app->staticpath);

  if (app->staticpath[staticpath_len - 1] != '/')
    staticpath = join(app->staticpath, "");
  else
    staticpath = strdup(app->staticpath);

  if (startswith(realpath, staticpath)) {
    memmove(realpath, realpath + strlen(staticpath), strlen(realpath));
    if (realpath[strlen(realpath) - 1] == '/')
      goto SKIP;

    char *fp = join(app->staticdir, realpath);
    if (contains(fp, '\\') || strstr(fp, "..") != NULL) {
      free(fp);
      goto SKIP;
    }

    if (!file_canread(fp)) {
      free(fp);
      goto SKIP;
    }

    res_sendfile(res, fp);
    res->code = 200;
    found = true;
    free(fp);
  }

SKIP:
  free(staticpath);
  free(realpath);

DONE:
  if (!found)
    app->allroute(req, res);
}
