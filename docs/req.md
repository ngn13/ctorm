# Request functions
> [!IMPORTANT]
> You should **NOT** `free()` any data returned by this functions
> **UNLESS** it's explicitly told to do so.

> [!WARNING]
> Most of these functions are macros, and will only work if the
> `req_t` pointer is named `req`, otherwise you should directly
> use the original functions.

### Request path
There are two different sections of the `req_t` which you can
use to access the request path, however you should not directly modify
these sections:
```c
info("URL encoded full path with the queries: %s", req->encpath);
info("URL decoded full path without the queries: %s", req->path);
```

### Request method
```c
char* method = REQ_METHOD(); // do not free or directly modify
if(strcmp(method, "GET")==0)
    info("This is a GET request");
```

### Request address
To access the source TCP IPv4/IPv6 address:
```c
info("Request from %s", req->addr);
```

### Cancelling the request
If you are using the request object inside a middleware handler,
you can cancel the request, making so will prevent it from reaching
the other middlewares and the other routes:
```c
REQ_CANCEL();
```
Or you can cancel it manually:
```c
req->cancel = true;
```

### Working with request body
To access the HTTP request body, you can read it into a buffer
you provide:
```c
// get the body size
uint64_t size = REQ_BODY_SIZE();
// uint64_t size = req_body_size(req);

// check if body size is valid
if(size == 0){
    error("request does not contain a body");
    return;
}

// allocate buffer for the body data
char *body = malloc(size);

// read "size" bytes of body into the "buffer"
REQ_BODY(body, size);
// req_body(req, body, size);
```

ctrom also contains few helper functions to work with certain
body formats:
```c
// parse the form encoded body
enc_url_t *form = REQ_FORM();
// enc_url_t *form = req_form(req);
char *username = enc_url_get(form, "username"); // do not free or directly modify
enc_url_free(form); // "username" now points to an invalid address

// parse the JSON encoded body
cJSON *json = REQ_JSON();
// cJSON *json = req_json(req);
cJSON *un_item = cJSON_GetObjectItem(json, "username");
char *un = cJSON_GetStringValue(un_item); // do not free or directly modify
enc_json_free(json); // "un" and "un_item" now points to an invalid address
```

### Request queries
To get URL decoded request queries, you can use the `REQ_QUERY` macro or the
`req_query` function:
```c
char *username = REQ_QUERY("username"); // do not free or directly modify
// char *username = req_query(req, "username");

if(NULL == username){
    error("username query is not specified");
    return;
}

info("username: %s", username);
```

### Request headers
To get HTTP headers, you can use the `REQ_HEADER` macro or the
`req_header` function, please note that HTTP headers are case-insensitive.

Also, if the client sent multiple headers with the same name, this macro/function
will return the first one in the header list.
```c
char* agent = REQ_GET("User-Agent");
// char *agent = req_get(req, "User-Agent");

if(NULL == agent){
    error("user-agent header is not set");
    return;
}

info("user-agent is %s", agent);
```
