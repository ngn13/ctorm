#pragma once

#include "config.h"
#include "http.h"
#include "pool.h"

#include "req.h"
#include "res.h"

#ifdef CTORM_EXPORT

typedef void *ctorm_app_t;

#endif

// route handler function type
typedef void (*ctorm_route_t)(ctorm_req_t *, ctorm_res_t *);

#ifndef CTORM_EXPORT
/*

 * routemap defines a single route
 * it stores route's path, HTTP request method, handler etc.

*/
typedef struct ctorm_routemap {
  struct ctorm_routemap *next;
  bool                   is_middleware;
  bool                   is_all;
  char                  *path;
  method_t               method;
  ctorm_route_t          handler;
} ctorm_routemap_t;

/*

 * ctorm web app structure

 * this the main structure that stores all the routes,
 * configuration and other important info

*/
typedef struct ctorm_app {
  ctorm_routemap_t *middleware_maps; // middleware map
  ctorm_routemap_t *route_maps;      // route map
  char             *staticpath;      // static directory serving path
  char             *staticdir;       // static directory
  ctorm_route_t     allroute;        // all handler route (see app_all())
  bool              running;         // is the app running?
  pool_t           *pool;            // thread pool for the app
  pthread_mutex_t   request_mutex;   // mutex used to lock request threads

  ctorm_config_t *config;            // app configuration
  bool            is_default_config; // using the default configuration?
} ctorm_app_t;

void ctorm_app_route(ctorm_app_t *app, ctorm_req_t *req, ctorm_res_t *res); // internal route handler

#endif

#define CTORM_VERSION "1.5"

ctorm_app_t *ctorm_app_new(ctorm_config_t *config); // creates a new application, use app_free() when you are done
bool         ctorm_app_run(ctorm_app_t *app, const char *host); // start the app on the specified host
bool         ctorm_app_add(
            ctorm_app_t *app, char *method, bool is_middleware, char *path, ctorm_route_t handler); // add a new route
void ctorm_app_all(ctorm_app_t *app, ctorm_route_t handler); // handler for all the unhandled routes
bool ctorm_app_static(
    ctorm_app_t *app, char *path, char *dir); // serve the static content in the dir on specifed the path
void ctorm_app_free(ctorm_app_t *app);        // frees and closes the resources of the created application
