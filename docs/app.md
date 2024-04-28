# App functions
### Route functions 
Functions used for routing should follow this structure:
```c
void handle_post(req_t*, res_t*);
```

### Simple routing
```c
// routes only will be called if they 
// exactly match
GET("/", get_index);
PUT("/", put_index);
HEAD("/", head_index);
POST("/", post_index);
DELETE("/", delete_index);
OPTIONS("/", options_index); 

// you can also setup routes with regex
GETR("123$", 123_route) // handle routes that end with 123
```

### Set static directory
```c
// static files be served at '/static' path, 
// from the './files' direcotry
app_static(app, "/static", "./files");

// if your app_t object is called 'app', then
// you can use the macro
APP_STATIC("/static", "./files");
```

### Setup 404 (all) route 
By default, routes that does not match with any other will be redirected
to a 404 page, you set a custom route for this:
```c
app_all(app, all_route);
// or you can use the macro
APP_ALL(all_route);
```
