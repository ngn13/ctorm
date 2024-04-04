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
#include "../include/log.h"
#include "../include/req.h"
#include "../include/res.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include <stdio.h>

app_t* app;
void app_init(){
  app = malloc(sizeof(app_t));
  app->allroute = app_404;
  app->staticdir = NULL;
  app->staticpath = NULL;
  app->map_size = 0;
  app->maps = NULL;
  setbuf(stdout, NULL);
}

bool app_run(const char* addr){
  char *save, *ip = NULL, *ports = NULL;
  char* addrcpy = strdup(addr);
  int port = -1;

  ip = strtok_r(addrcpy, ":", &save);
  if(NULL == ip){
    errno = BadAddress;
    goto FAIL;
  }

  ports = strtok_r(NULL, ":", &save);
  if(NULL == ports){
    errno = BadAddress;
    goto FAIL;
  }

  port = atoi(ports);
  if(port > 65535 || port <= 0){
    errno = BadPort;
    goto FAIL;
  }

  info("Starting the application on http://%s:%d", ip, port);
  if(start_socket(app, ip, port)){
    free(addrcpy);
    return true;
  }

FAIL:
  free(addrcpy);
  return false;
}

void app_static(char* path, char* dir){
  app->staticpath = path;
  app->staticdir = dir;
}

void app_all(route handler){
  app->allroute = handler;
}

bool app_add(char* method, bool regex, char* path, route handler){
  app->map_size++;
  if(NULL == app->maps)
    app->maps = malloc(sizeof(routemap));
  else 
    app->maps = realloc(app->maps, 
        sizeof(routemap)*app->map_size);

  if(NULL == app->maps){
    errno = AllocFailed;
    return false;
  }

  app->maps[app->map_size-1].method = http_methodid(method);
  app->maps[app->map_size-1].handler = handler;
  app->maps[app->map_size-1].regex = regex;
  app->maps[app->map_size-1].path = path;
  return true;
}

void app_404(req_t* req, res_t* res){
  res_set(res, "Content-Type", "text");
  res_send(res, "Not Found");
  res->code = 404;
}

void app_route(req_t* req, res_t* res){
  bool found = false;
  regex_t regex;

  for(int i = 0; i < app->map_size; i++){
    if(app->maps[i].method != req->method)
      continue;

    if(app->maps[i].regex)
      goto REGEX;

    if(eq(app->maps[i].path, req->path)){
      app->maps[i].handler(req, res);
      found = true;
    }
    continue;

  REGEX:
    if(regcomp(&regex, app->maps[i].path, 0)){
      error("Skipping bad route: %s", app->maps[i].path);
      continue;
    }

    if(regexec(&regex, req->path, 0, NULL, 0)==REG_NOMATCH){
      regfree(&regex);
      continue;
    }

    regfree(&regex);
    app->maps[i].handler(req, res);
    found = true;
  }

  if(found)
    goto DONE;

  if(NULL==app->staticpath || NULL==app->staticdir)
    goto DONE;

  char *staticpath, *realpath = strdup(req->path);
  int staticpath_len = strlen(app->staticpath);

  if(app->staticpath[staticpath_len-1] != '/')
    staticpath = join(app->staticpath, "");
  else 
    staticpath = strdup(app->staticpath);

  if(startswith(realpath, staticpath)){
    memmove(realpath, realpath+strlen(staticpath), strlen(realpath));
    if(realpath[strlen(realpath)-1]=='/')
      goto SKIP;

    char* fp = join(app->staticdir, realpath);
    if(contains(fp, '\\') || strstr(fp, "..")!=NULL){
      free(fp);
      goto SKIP;
    }
    
    if(!file_canread(fp)){
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
  if(!found)
    app->allroute(req, res);
}
