# Response functions 
> [!IMPORTANT]  
> You should **NOT** `free()` any data returned by this functions
> **UNLESS** it's explicitly told to do so.

> [!WARNING]
> Most of these functions are macros, and will only work if the
> `res_t` pointer is named `res`, otherwise you should directly
> use the original functions.

### Setting the response code
The default response code is `200 OK`. You can change this
using the `RES_CODE()` macro:
```c
RES_CODE(403);
```
Or you can directly modify the response code:
```c
res->code = 403;
```

### Working with the response body 
There are few different ways to work with the response body:
```c
// you can use local data, it will be copied to heap
// when you call res_send()
RES_SEND("hello world!");

// if you have raw data (not null terminated), use the
// function instead of the macro, and specify the size
// at the end
char raw[128];
...
res_send(res, data, sizeof(raw));

// if the data is null terminated, you can set the size
// to 0, and the size will be calculated with strlen()
// internally
res_send(res, "hello world!", 0);
```

If you have formatted text, you can use the formatted
functions to make your life easier:
```c
// just like with `res_send`, you can use local data
// this macro/function will overwrite the response body
RES_FMT("username: %s", username);
res_fmt(res, "username: %s", username);

// this macro/function will append to the response body
RES_ADD("age: %d", age);
res_add(res, "age: %d", age);
```

You can also send JSON data with response body using `cJSON`
objects:
```c
cJSON *json = cJSON_CreateObject();
cJSON_AddStringToObject(json, "username", "John");

// send the JSON data
RES_JSON(json);

// same thing without using the macro
res_json(res, json);
```

If for whatever reason, you want to completely clear the
response body:
```c
RES_CLEAR();

// without using the macro
res_clear(res);
```

### Sending a file
Using relative or absolute paths, you can send files with
the response using the `RES_SENDFILE` macro or the `res_sendfile`
function, `Content-Type` will be set for `html`, `css`, `js` and `json`
files based on the extension, for any other file type the `Content-Type` 
will be set to `text/plain` and you may need to manually set it.
```c
RES_SENDFILE("files/index.html");
res_sendfile(res, "files/index.html");
```

### Working with headers
To set a header, you can use the `RES_SET` macro or the `res_set`
function:
```c
// just like the other body functions, you can use local data
RES_SET("Cool", "yes");
res_set(res, "Cool", "yes");
```

To remove a header, you can use `RES_DEL` macro or the `res_del`
function:
```c
RES_DEL("Cool");
res_del(res, "Cool");
```

### Redirecting
To redirect the client to another page or a URL, you can use the
`RES_REDIRECT` macro or the `res_redirect` function:
```c
RES_REDIRECT("/login");
res_redirect(res, "/login");
```
