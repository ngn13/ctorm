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
req->cancel = true;
```

### Working with request body
HTTP request body is non-null terminated (not printable), raw
data section of the `req_t` structure, you can directly access
it with the help of the `bodysize` section, however **you should
not directly modify it**.

Before using the request body, you should also make sure that the
request actually contains a body:
```c
if(req->bodysize == 0 || req->body == NULL){
    error("request does not contain a body");
    return;
}

// the following will (potentially) read arbitrary data
// because "req->body" is not null terminated
// info("body: %s", req->body);

char body_copy[req->bodysize];
memcpy(body_copy, req->body, req->bodysie);
```

To get the null terminated body (printable), you can use the `REQ_BODY`
macro or the `req_body` function:
```c
size_t bodysize = REQ_BODY_SIZE();
if(bodysize == 0){
    error("request does not contain a body");
    return;
}

char body[bodysize];
REQ_BODY(body);

info("body: %s", body);
```

ctrom also contains few helper functions to work with certain
body formats:
```c
// parse the form encoded body
form_t *form = REQ_FORM();
//form_t *form = req_form(req);
char *username = req_form_get(form, "username"); // do not free or directly modify
req_form_free(form); // "username" now points to an invalid address 

// parse the JSON encoded body
cJSON *json = REQ_JSON();
//cJSON *json = req_json(req);
cJSON *useritem = cJSON_GetObjectItem(json, "username");
char *username = cJSON_GetStringValue(useritem); // do not free or directly modify
req_json_free(json); // "username" and "useritem" now points to an invalid address
```

### Request queries
To get URL decoded request queries, you can use the `REQ_QUERY` macro or the
`req_query` function:
```c
char *username = REQ_QUERY("username"); // do not free or directly modify
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
char* agent = REQ_HEADER("User-Agent");
if(NULL == agent){
    error("user-agent header is not set");
    return;
}

info("user-agent is %s", agent);
```
