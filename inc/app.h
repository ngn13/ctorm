/*!

 * @file
 * @brief Header file for the web server application functions and definitions

*/
#pragma once

#include "config.h"
#include "http.h"
#include "pool.h"

#include "req.h"
#include "res.h"

#ifdef CTORM_EXPORT

typedef void *ctorm_app_t;

#endif

/*!

 * @brief Route handler function

 * Type for the route handler function, which handles HTTP requests
 * for a route, you can register this handler to your web server
 * application using @ref ctorm_app_add

*/
typedef void (*ctorm_route_t)(ctorm_req_t *, ctorm_res_t *);

#ifndef CTORM_EXPORT

/*!

 * @brief Stores information about an added route

 * Stores information about a route that has been added to
 * the application's route list using @ref ctorm_app_add

*/
typedef struct ctorm_routemap {
  struct ctorm_routemap *next;
  bool                   is_middleware;
  char                  *path;
  method_t               method;
  ctorm_route_t          handler;
} ctorm_routemap_t;

#define ctorm_routemap_is_all(route) (route->method == -1)

/*!

 * @brief Stores information about the web server

 * Stores information about the web server such as the application's
 * state (running or not) and application's routes and middlewares, you can
 * create a new application using @ref ctorm_app_new

*/
typedef struct ctorm_app {
  ctorm_routemap_t *middleware_maps; /// middleware map
  ctorm_routemap_t *route_maps;      /// route map
  char             *static_path;     /// static directory serving path
  char             *static_dir;      /// static directory
  ctorm_route_t     all_route;       /// all handler route (see app_all())
  bool              running;         /// is the app running?
  pool_t           *pool;            /// thread pool for the app
  pthread_mutex_t   request_mutex;   /// mutex used to lock request threads

  ctorm_config_t *config;            /// app configuration
  bool            is_default_config; /// using the default configuration?
} ctorm_app_t;

/*!

 * Routes a given request to the correct middlewares and routes

 * @param[in] app ctorm server application
 * @param[in] req HTTP request to route
 * @param[in] res HTTP response for the request

*/
void ctorm_app_route(ctorm_app_t *app, ctorm_req_t *req, ctorm_res_t *res);

#endif

#define CTORM_VERSION "1.5"

/*!

 * Creates a new @ref ctorm_app_t structure, which is the web server
 * structure, when you are done you should free the returned @ref ctorm_app_t
 * pointer using @ref ctorm_app_free

 * @param[in] config Configuration for the web server, set it to NULL to use the
 *                   default configuration
 * @return Pointer for the created web server application

*/
ctorm_app_t *ctorm_app_new(ctorm_config_t *config);

/*!

 * Start the web server on a provided host address, note that this will
 * hang the current thread until the server is stopped by @ref ctorm_app_stop

 * @param[in] app  ctorm server application
 * @param[in] host Host address that the web server should start on
 * @return Returns false if an error occurs, you can obtain the error from the errno

*/
bool ctorm_app_run(ctorm_app_t *app, const char *host);

/*!

 * Stop the web server, this will have no effect if you did not start the web
 * server with @ref ctorm_app_run

 * param[in] app ctorm server application
 * @return Returns false if an error occurs, you can obtain the error from the errno

*/
bool ctorm_app_stop(ctorm_app_t *app);

/*!

 * Add a route handler to the web server, which will handle all the requests
 * for a specific route

 * param[in] app           ctorm server application
 * param[in] method        HTTP method for the route
 * param[in] is_middleware Specifies if the route is a middleware or not
 * param[in] path          Path for the route
 * param[in] handler       Route handler function
 * @return Returns false if an error occurs, you can obtain the error from the errno

*/
bool ctorm_app_add(ctorm_app_t *app, char *method, bool is_middleware, char *path, ctorm_route_t handler);

/*!

 * Specify a route handler for all the unhandled routes

 * param[in] app     ctorm server application
 * param[in] handler Route handler function

*/
void ctorm_app_all(ctorm_app_t *app, ctorm_route_t handler);

/*!

 * Serve static content from a directory under the specified route

 * param[in] app  ctorm server application
 * param[in] path Path for the route
 * param[in] dir  Directory that contains the static content
 * @return Returns false if an error occurs, you can obtain the error from the errno

*/
bool ctorm_app_static(ctorm_app_t *app, char *path, char *dir);

/*!

 * Free the memory used by the web server application, note that
 * the pointer for the server application will point to invalid
 * memory after this operation, you should not pass this pointer to
 * any app functions

 * param[in] app ctorm server application

*/
void ctorm_app_free(ctorm_app_t *app);
