#pragma once

#include "errors.h"
#include "http.h"
#include "log.h"
#include "req.h"
#include "res.h"

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

void app_init();
bool app_run(const char *);
bool app_add(char *, bool, char *, route_t);
void app_route(req_t *, res_t *);
void app_static(char *, char *);
void app_404(req_t *, res_t *);
void app_all(route_t);

#define GET(path, func) app_add("GET", false, path, func)
#define PUT(path, func) app_add("PUT", false, path, func)
#define HEAD(path, func) app_add("HEAD", false, path, func)
#define POST(path, func) app_add("POST", false, path, func)
#define DELETE(path, func) app_add("DELETE", false, path, func)
#define OPTIONS(path, func) app_add("OPTIONS", false, path, func)

#define GETR(path, func) app_add("GET", true, path, func)
#define PUTR(path, func) app_add("PUT", true, path, func)
#define HEADR(path, func) app_add("HEAD", true, path, func)
#define POSTR(path, func) app_add("POST", true, path, func)
#define DELETER(path, func) app_add("DELETE", true, path, func)
#define OPTIONSR(path, func) app_add("OPTIONS", true, path, func)
