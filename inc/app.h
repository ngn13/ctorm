/*!

 * @file
 * @brief Header file for the web server application functions and definitions

*/
#pragma once

#include "config.h"

#include "http.h"
#include "pair.h"
#include "pool.h"
#include "util.h"

#include "req.h"
#include "res.h"

#ifdef CTORM_EXPORT

typedef void ctorm_app_t;

#endif

/*!

 * @brief Route handler function

 * Type for the route handler function, which handles HTTP requests for a route,
 * you can register this handler to your web server application using @ref
 * ctorm_app_add

*/
typedef void (*ctorm_route_t)(ctorm_req_t *, ctorm_res_t *);

#ifndef CTORM_EXPORT

struct ctorm_route {
  char               *path;
  bool                all;
  ctorm_http_method_t method;
  ctorm_route_t       handler;
  struct ctorm_route *next;
};

typedef struct ctorm_app {
  bool running; // is the app running?
  int  error;   // last error the app encountered

  pthread_t       thread;    // thread the app is running in
  pthread_mutex_t req_mutex; // locked before processing a request
  pthread_mutex_t mod_mutex; // locked before modifying the app

  // routes
  ctorm_route_t       default_route;
  struct ctorm_route *routes;

  cu_str_t static_path; // static route path
  cu_str_t static_dir;  // static route directory

  ctorm_pair_t *locals; // local vars (passed to every request)
  ctorm_pool_t *pool;   // web server thread pool

  ctorm_config_t *config;            // web server configuration
  bool            is_default_config; // using the default configuration?

  struct ctorm_app *next;
} ctorm_app_t;

void ctorm_app_route(ctorm_app_t *app, ctorm_req_t *req, ctorm_res_t *res);

#endif

#define CTORM_VERSION "1.8"

/*!

 * Create a new ctorm web server with the given configuration. If no
 * configuration is provided, default configuration will be used. The returned
 * @ref ctorm_app_t pointer should be freed using @ref ctorm_app_free

 * @param[in] config: Configuration for the web server, set it to NULL to use
 *                    the default configuration
 * @return    Pointer for the created web server application

*/
ctorm_app_t *ctorm_app_new(ctorm_config_t *config);

/*!

 * Start the provided web server on the provided host address. Please note that
 * this will hang the current thread until the server is stopped by @ref
 * ctorm_app_stop

 * @param[in] app:  ctorm server application
 * @param[in] addr: Host address that the web server should start on
 * @return          Returns false if an error occurs, you can obtain the error
 *                  from the errno

*/
bool ctorm_app_run(ctorm_app_t *app, const char *addr);

/*!

 * Stop the provided web server. This function will do nothing if you did not
 * start the the web server with @ref ctorm_app_run. If you did, then this
 * function will cause @ref ctorm_app_run to return

 * @param[in] app: ctorm server application
 * @return    Returns false if an error occurs, you can obtain the error from
 *            the errno

*/
bool ctorm_app_stop(ctorm_app_t *app);

/*!

 * Get or a set local variable. These locals will be copied to every single
 * request, making them accessible from every route.

 * @param[in] app:   ctorm server application
 * @param[in] name:  Local name
 * @param[in] value: Local value
 * @return    Returns false if an error occurs, you can obtain the error from
 *            the errno

*/
bool ctorm_app_local(ctorm_app_t *app, char *name, void *value);

/*!

 * Add a route handler to the web server, which will handle all the requests
 * for a specific route

 * @param[in] app:     ctorm server application
 * @param[in] method:  HTTP method for the route
 * @param[in] path:    Path for the route
 * @param[in] handler: Route handler function
 * @return    Returns false if an error occurs, you can obtain the error from
 *            the errno

*/
bool ctorm_app_add(
    ctorm_app_t *app, int method, char *path, ctorm_route_t handler);

/*!

 * Set the default route handler for all the unhandled routes

 * @param[in] app:     ctorm server application
 * @param[in] handler: Route handler function

*/
void ctorm_app_default(ctorm_app_t *app, ctorm_route_t handler);

/*!

 * Serve static content from a directory under the specified route

 * @param[in] app:  ctorm server application
 * @param[in] path: Path for the route
 * @param[in] dir:  Directory that contains the static content
 * @return    Returns false if an error occurs, you can obtain the error from
 *            the errno

*/
bool ctorm_app_static(ctorm_app_t *app, char *path, char *dir);

/*!

 * Free the memory used by the provided web server. Note that the provided @ref
 * ctorm_app_t web server pointer, will no longer be valid after this operation.

 * @param[in] app: ctorm server application

*/
void ctorm_app_free(ctorm_app_t *app);
