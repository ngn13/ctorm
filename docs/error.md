# Error functions

### Checking the error code

When a function fails, depending on the return value, you may receive a `false`,
`NULL` or a negative number return. To learn what went wrong and why the
function failed, you can use check the `errno`:

```c
#include <errno.h>
...
if(!ctorm_app_run(app, argv[1])){
    if(errno == CTORM_BAD_HOST)
        ctorm_fail("you specified an invalid host address");
    else
        ctorm_fail("something else went wrong: %d", errno);
}
```

See [errors.h](../inc/error.h) for the full list of error codes.

### Getting the error description

To get the string description of an error, you can use `ctorm_error`:

```c
if(!ctorm_app_run(app, "0.0.0.0:8080"))
    ctorm_fail("something went wrong: %s", ctorm_error());
```

Or you can get the description of a specific error code:

```c
ctorm_info("CTORM_BAD_HOST: %s", ctorm_error_from_str(CTORM_BAD_HOST));
```

### Getting the error details

You can also get the details of an error using the To get the details of an
error, which is essentially the saved, previous `errno`, you can use the
`ctorm_details` macro:

```c
if (!ctorm_app_run(app, "0.0.0.0:8080")) {
    ctorm_fail("failed to start the app: %s", ctorm_error());
    ctorm_fail("details: %s", ctorm_details());
}
```

Unlike `ctorm_error`, you need access to the `ctorm_app_t` pointer to use this
macro. You can also use the function:

```c
ctorm_app_t *my_app = ctorm_app_new(NULL);
...
ctorm_fail("details: %s", ctorm_error_details(my_app));
```
