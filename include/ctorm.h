#pragma once

#define CTORM_VERSION "1.5"

#include "config.h"
#include "http.h"
#include "pool.h"
#include "req.h"
#include "res.h"

// ###################
// ## route handler ##
// ###################
typedef void (*route_t)(req_t *, res_t *);

// #############################
// ## route map (linked list) ##
// #############################
typedef struct routemap_t {
  struct routemap_t *next;
  bool               is_middleware;
  bool               is_all;
  char              *path;
  method_t           method;
  route_t            handler;
} routemap_t;

// ###################
// ## app structure ##
// ###################
typedef struct app_t {
  routemap_t     *middleware_maps; // middleware map
  routemap_t     *route_maps;      // route map
  char           *staticpath;      // static directory serving path
  char           *staticdir;       // static directory
  route_t         allroute;        // all handler route (see app_all())
  bool            running;         // is the app running?
  pool_t         *pool;            // thread pool for the app
  pthread_mutex_t request_mutex;   // mutex used to lock request threads

  app_config_t *config;            // app configuration
  bool          is_default_config; // using the default configuration?
} app_t;

app_t *app_new(app_config_t *config);         // creates a new application, use app_free() when you are done
bool   app_run(app_t *app, const char *addr); // start the app on the specified address
bool   app_add(app_t *app, char *method, bool is_middleware, char *path, route_t handler); // add a new route
bool   app_static(app_t *app, char *path, char *dir); // serve the static content in the dir on specifed the path
void   app_all(app_t *app, route_t handler);          // handler for all the unhandled routes
void   app_route(app_t *app, req_t *request, res_t *response); // internal route handler
void   app_404(req_t *request, res_t *response);               // internal 404 route handler, default for app_all()
void   app_free(app_t *app); // frees and closes the resources of the created application
