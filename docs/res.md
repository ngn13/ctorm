# Response functions 
> [!IMPORTANT]  
> You should **NOT** `free()` any data returned by this functions
> **UNLESS** it's explicitly told to do so 

### Setting the status code  
```c
res->code = 403;
```

### Sending simple strings
```c
// you can use local data, it will be copied to heap
// when you call res_send()
res_send(res, "hello world!");

// or you can use the macro
RES_SEND("hello world!");
```

### Sending a file
```c
res_sendfile(res, "files/index.html");

// or you can use the macro
RES_SENDFILE("files/index.html");
```

### Set a header
```c
// again, you can use local data
res_set(res, "Cool", "yes");

// or you can use the macro
RES_SET("Cool", "yes");
```
