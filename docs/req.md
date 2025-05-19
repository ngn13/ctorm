# Request functions

You should **NOT** `free()` any data returned by this functions **UNLESS** it's
explicitly told to do so.

Most of these functions are macros, and will only work if the `ctorm_req_t`
pointer is named `req`, otherwise you should directly use the original
functions.

### Request target

There are different fields in the `ctorm_req_t` object which you can use to
access the different target components:

```c
ctorm_info("HTTP request target: %s", req->target);
ctorm_info("Target host: %s", req->host);
ctorm_info("Target path: %s", req->path);
```

However **you should not directly modify any of these fields**.

### Request method

```c
char* method = REQ_METHOD(); // do not free or directly modify
if(strcmp(method, "GET")==0)
    info("This is a GET request");
```

### Request address

To access the client TCP IPv4/IPv6 address:

```c
char ipbuf[INET6_ADDRSTRLEN+1];
REQ_IP(ipbuf)
info("Request from %s", ipbuf);

// or...

char *ipbuf = REQ_IP(NULL);
info("Request from %s", ipbuf);
free(ipbuf);
```

You can also directly access the `struct sockaddr` structure with `REQ_ADDR()`.
However you should not modify this structure.

### Cancelling the request

If you are using the request object inside a route, you can cancel the request,
which will prevent it from reaching the next routes:

```c
REQ_CANCEL();
```

Or you can cancel it manually:

```c
req->cancel = true;
```

### Working with request body

To access the HTTP request body, you can read it into a buffer you provide:

```c
// get the body size
int64_t size = REQ_BODY_SIZE();
// int64_t size = req->body_size;

// check if body size is valid
if(size == 0){
    ctorm_fail("request does not contain a body");
    return;
}

// allocate buffer for the body data
char *body = malloc(size);

// read "size" bytes of body into the "buffer"
REQ_BODY(body, size);
// ctorm_req_body(req, body, size);
```

ctrom also contains few helper functions to work with certain body formats:

```c
// parse the form encoded body
ctorm_query_t *form = REQ_FORM();
// ctorm_query_t *form = ctorm_req_form(req);
char *username = ctorm_query_get(form, "username"); // do not free or directly modify
ctorm_url_free(form); // "username" now points to an invalid address

// parse the JSON encoded body
cJSON *json = REQ_JSON();
// cJSON *json = ctorm_req_json(req);
cJSON *un_item = cJSON_GetObjectItem(json, "username");
char *un = cJSON_GetStringValue(un_item); // do not free or directly modify
ctorm_json_free(json); // "un" and "un_item" now points to invalid addresses
```

### Request queries

To get URL decoded request queries, you can use the `REQ_QUERY` macro or the
`req_query` function:

```c
char *username = REQ_QUERY("username"); // do not free or directly modify
// char *username = ctorm_req_query(req, "username");

if(NULL == username){
    ctorm_fail("username query is not specified");
    return;
}

ctorm_info("username: %s", username);
```

### Request parameters

If the route uses a URL parameter (see [app documentation](app.md) for more
information) then you can access this parameter by it's name:

```c
char *lang = REQ_PARAM("lang");
// char *lang = ctorm_req_param(req, "lang");

if(NULL == lang){
    ctorm_warn("no language specified, using the default");
    lang = "en";
}

ctorm_info("language: %s", lang);
```

### Request headers

To get HTTP headers, you can use the `REQ_GET` macro or the `ctorm_req_get`
function, please note that HTTP headers are case-insensitive.

Also it's worth noting if the client sent multiple headers with the same name,
this macro/function will return the last one.

```c
char* agent = REQ_GET("user-agent");
// char *agent = ctorm_req_get(req, "user-agent");

if(NULL == agent){
    ctorm_fail("user-agent header is not set");
    return;
}

ctorm_info("user-agent is %s", agent);
```

### Request locals

You may want to pass variables around different routes that handle the same
request. To do this, you can use request locals, inspired by
[fiber's ctx locals](https://docs.gofiber.io/api/ctx/#locals).

Here is a route that creates a new local named "username":

```c
char *username = REQ_QUERY("username");

if(NULL == username){
    RES_SEND("please specify a username");
    REQ_CANCEL();
    return;
}

for(char *c = username; *c != 0; c++)
    *c = tolower(*c);

REQ_LOCAL("username", username);
// ctorm_req_local(req, "username", username, NULL);
```

In an another route that handle the same request after the previous route, you
can access the "username" local:

```c
char *username = REQ_LOCAL("username");
// char *username = ctorm_req_local(req, "username", NULL);
```

If you want to pass a variable to all routes, you can use `ctorm_app_local`. See
the [app documentation](app.md).
