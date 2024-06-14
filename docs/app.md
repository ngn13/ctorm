# App functions
### Configuration
Before creating an application, you can create a custom
configuration:
```c
app_config_t config;
```
But before using it you should initialize it:
```c
app_config_new(&config);
```
This will set all the default values, you can modify these by
directly accessing them through the `app_config_t` structure,
for example:
```c
// disable request/response logging
config.disable_logging = false;
```

### Managing the application
To create an application:
```c
app_t *app = app_new(&config);
```
If you don't have a custom configuration and you want
to use the default configuration:
```c
app_t *app = app_new(NULL);
```
And to start the application:
```c
app_run(app, "0.0.0.0:8080")
```
This will start the application on port 8080, all interfaces,
after the app stops, you should clean up the `app_t` pointer
to free all the resources, to do this:
```c
app_free(app);
```

### Simple routing
Handlers used for routing should follow this structure:
```c
void route(req_t*, res_t*);
```
The `req_t` pointer points to the request object, and the `res_t`
pointer points to the response object. To learn what you can do with them,
check out the [request](req.md) and [response](res.md) documentation.

To setup routes, you can use these simple macros:
```c
 // get_index will handle any GET request for /
GET(app, "/", get_index);

// set_route will handle any PUT request for any routes
// under the `/set` route
PUT(app, "/set/*", set_route);

// top_secret will handle any request which is under the `/top`
// route which contains the `/secret` sub-route
ALL(app, "/top/*/secret", top_secret);

// similar examples
HEAD(app, "/", head_index);
POST(app, "/", post_index);
DELETE(app, "/", delete_index);
OPTIONS(app, "/", options_index);
```

### Middleware
Middleware handlers have the exact same structure with the routes,
however they have different macros:
```c
MIDDLEWARE_ALL(app, "/auth/*", auth_check);
MIDDLEWARE_GET(app, "/", get_index_mid);
MIDDLEWARE_PUT(app, "/", put_index_mid)
MIDDLEWARE_HEAD(app, "/", head_index_mid)
MIDDLEWARE_POST(app, "/", post_index_mid)
MIDDLEWARE_DELETE(app, "/", delete_index_mid)
MIDDLEWARE_OPTIONS(app, "/", options_index_mid)
```

### Set static directory
To setup a static route you can use the `app_static` function,
**please note that ctorm only supports a single static route.**
```c
// static files be served at '/static' path,
// from the './files' directory
app_static(app, "/static", "./files");
```

### Setup 404 (all) route
By default, routes that does not match with any other will be redirected
to a 404 page, you set a custom route for this:
```c
app_all(app, all_route);
```

### Error handling
When a function fails, depending on the return value, you may receive a `false`
or a `NULL` return. To learn what went wrong and why the function failed, you can
use the `app_geterror` function, here is an example:
```c
if(!app_run(app, "127.0.0.1:8080"))
    error("something went wrong: %s", app_geterror());
```
The error code also will be set the with the `errno` variable:
```c
#include <errno.h>
...
if(!app_run(argv[1])){
    if(errno == BadAddress)
        error("you specified an invalid address");
    else
        error("failed to start the app: %s", app_geterror());
}
```
