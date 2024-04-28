# Request functions 
> [!IMPORTANT]  
> You should **NOT** `free()` any data returned by this functions
> **UNLESS** it's explicitly told to do so 

### Check if request has a body
```c
bool has_body = req_has_body(req);
if(!has_body)
  // request does not has a body
else 
  // body is avaliable
```

### Get request size  
```c
int size = req->bodysz;
```

### Get request body
Request body is a character buffer **THAT DOES NOT END WITH A NULL TERMINATOR.**
Body is completely plain binary data. 
```c
char* body = req->body;
// this will read arbitrary data, again the data does 
// not end with a null terminator 
printf("Body: %s\n", req->body);
```

### Get request body (string)
To get the body that ends with a null terminator, use this function:
```c
char* body = req_body(req); 
// req_body creates a copy of the req->body 
// on the heap, so you need to free it when you 
// are done with it
free(req->body);
```

### Get method from request 
```c
char* method = req_method(req);
```

### Get a header of a request
```c
char* agent = req_header(req, "User-Agent");
if(NULL == agent){
  // user-agent header is not set
  return;
}

// or you can use the macro
char* agent = REQ_HEADER("User-Agent");
```

### Get request URL query/parameter
```c
char* msg = req_query(req, "msg");
if(NULL == msg){
  // msg parameter is specified
  return;
}

// or you can use the macro
char* agent = REQ_QUERY("msg");
```

### Parse URL encoded form data
Ctorm can parse simple URL encoded form data from the request body:
```c
body_t* body = req_body_parse(req);
if(NULL == body)
  // body is empty or not parsable
```
To get a value of a key from this parsed data:
```c
char* msg = req_body_get(body, "msg");
if(NULL == msg)
  // msg key not set in the body
```
Do not forget the free the data when you are done with it:
```c
req_body_free(body);
```
