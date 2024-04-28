#pragma once

#include "errors.h"
#include "http.h"
#include "log.h"
#include "req.h"
#include "res.h"

#define CTORM_VERSION "1.3"

typedef void (*route_t)(req_t *, res_t *);

typedef struct routemap_t {
  struct routemap_t *next;
  bool regex;
  char *path;
  int method;
  route_t handler;
} routemap_t;

typedef struct app_t {
  routemap_t *maps;
  char *staticpath;
  char *staticdir;
  route_t allroute;
  int socket;
} app_t;

app_t *app_new();
bool app_run(app_t*, const char *);
bool app_add(app_t *, char *, bool, char *, route_t);
void app_route(app_t *, req_t *, res_t *);
void app_static(app_t *, char *, char *);
void app_404(req_t *, res_t *);
void app_all(app_t *, route_t);

#define GET(path, func) app_add(app, "GET", false, path, func)
#define PUT(path, func) app_add(app, "PUT", false, path, func)
#define HEAD(path, func) app_add(app, "HEAD", false, path, func)
#define POST(path, func) app_add(app, "POST", false, path, func)
#define DELETE(path, func) app_add(app, "DELETE", false, path, func)
#define OPTIONS(path, func) app_add(app, "OPTIONS", false, path, func)

#define GETR(path, func) app_add(app, "GET", true, path, func)
#define PUTR(path, func) app_add(app, "PUT", true, path, func)
#define HEADR(path, func) app_add(app, "HEAD", true, path, func)
#define POSTR(path, func) app_add(app, "POST", true, path, func)
#define DELETER(path, func) app_add(app, "DELETE", true, path, func)
#define OPTIONSR(path, func) app_add(app, "OPTIONS", true, path, func)
