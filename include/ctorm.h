#pragma once

#define CTORM_VERSION "1.4"

#include <event2/event.h>

#include "errors.h"
#include "http.h"
#include "log.h"
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
  bool               is_regex;
  char              *path;
  int                method;
  route_t            handler;
} routemap_t;

// #######################
// ## app configuration ##
// #######################
typedef struct app_config_t {
  bool     disable_logging; // disables request logging and the banner
  bool     handle_signal;   // disables SIGINT handler (which stops app_run())
  bool     server_header;   // disable sending the "Server: ctorm" header in the response
  long int tcp_timeout;     // sets the TCP socket timeout for sending and receiving
  int      pool_size;       // app threadpool size
} app_config_t;

void app_config_new(app_config_t *config);

// ###################
// ## app structure ##
// ###################
typedef struct app_t {
  struct event_base *base;       // libevent event base
  app_config_t      *config;     // app configuration
  routemap_t        *maps;       // route map
  char              *staticpath; // static directory serving path
  char              *staticdir;  // static directory
  route_t            allroute;   // all handler route (see app_all())
  bool               running;    // is the app running?
  pool_t            *pool;       // thread pool for the app
} app_t;

app_t *app_new(app_config_t *config);         // creates a new application, use app_free() when you are done
bool   app_run(app_t *app, const char *addr); // start the app on the specified address
bool   app_add(app_t *app, char *method, bool is_regex, char *path, route_t handler); // add a new route
void   app_all(app_t *app, route_t handler);          // handler for all the unhandled routes
void   app_static(app_t *app, char *path, char *dir); // serve the static content in the dir on specifed the path
void   app_route(app_t *app, req_t *request, res_t *response); // internal route handler
void   app_404(req_t *request, res_t *response);               // internal 404 route handler, default for app_all()
void   app_free(app_t *app); // frees and closes the resources of the created application

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
