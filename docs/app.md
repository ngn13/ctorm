# App functions

### Configuration

Before creating an application, you can create a custom configuration:

```c
ctorm_config_t config;
```

But before using it you should initialize it:

```c
ctorm_config_new(&config);
```

This will set all the default values, you can modify these by directly accessing
them through the `ctorm_config_t` structure, for example:

```c
// disable request/response logging
config.disable_logging = false;
```

### Managing the application

To create an application:

```c
ctorm_app_t *app = ctorm_app_new(&config);
```

If you don't have a custom configuration and you want to use the default
configuration:

```c
ctorm_app_t *app = ctorm_app_new(NULL);
```

And to start the application:

```c
ctorm_app_run(app, "0.0.0.0:8080")
```

This will start the application on port 8080, all interfaces, after the app
stops, you should clean up the `ctorm_app_t` pointer to free all the resources:

```c
ctorm_app_free(app);
```

### Simple routing

Routes are used for handling HTTP requests. A function can handle one or
multiple routes. Route functions follow this structure:

```c
void route(ctorm_req_t*, ctorm_res_t*);
```

The `ctorm_req_t` pointer points to the HTTP request object, and the
`ctorm_res_t` pointer points to the HTTP response object. To learn what you can
do with them, check out the [request](req.md) and
[response documentation](res.md).

To setup routes, you can use these simple macros:

```c
// get_index will handle any GET request for /
GET(app, "/", get_index);

// set_route will handle any PUT request for any routes under the `/set` route
PUT(app, "/set/%", set_route);

/* top_secret will handle any request which is under the `/top` route which
 * contains the `/secret` sub-route */
ALL(app, "/top/%/secret", top_secret);

// similar examples
HEAD(app, "/", head_index);
POST(app, "/", post_index);
DELETE(app, "/", delete_index);
OPTIONS(app, "/", options_index);
```

### URL parameters

You can use the `:` character to specify URL parameters in the routes:

```c
GET(app, "/blog/:lang/desc", get_blog_desc);
```

For example this route will act the same as `/blog/%/desc` route. However
whatever fills the percent sign (wildcard) will be used as the value of the
`lang` URL parameter.

Later during the handler call, URL parameters can be accessed using the request
pointer. See the [request documentation](req.md) for more information.

### Set static directory

To setup a static route you can use the `ctorm_app_static` function, **please
note that ctorm only supports a single static route**.

```c
// static files be served at '/static' path, from the './files' directory
ctorm_app_static(app, "/static", "./files");
```

### Default (404) route

Default route is the route used for all the unhandled requests. By default a 404
page route is used as the default route. You can change this by creating a
custom default route:

```c
ctorm_app_default(app, default_route);
```

This handler needs to follow the same route structure.

### Locals

If you want to pass a variable to all the routes, you can use locals:

```c
ctorm_app_local(app, "config", &config);
```

To access the local from a route, you can use `REQ_LOCAL` or `ctorm_req_local`.
See the [request documentation](req.md) for more information.
