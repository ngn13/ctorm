# Error functions
### Checking the error code
When a function fails, depending on the return value, you may receive a `false`
or a `NULL` return. To learn what went wrong and why the function failed, you can
use check the `errno`:
```c
#include <errno.h>
...
if(!ctorm_app_run(app, argv[1])){
    if(errno == BadAddress)
        ctorm_fail("you specified an invalid address");
    else
        ctorm_fail("something else went wrong: %d", errno);
    return EXIT_FAILURE;
}
```
See [errors.h](../inc/errors.h) for the full list of error codes.

### Getting the error description
To get the string description of an error, you can use `ctorm_geterror`:
```c
if(!ctorm_app_run(app, "0.0.0.0:8080")){
    ctorm_fail("something went wrong: %s", ctorm_geterror());
    return EXIT_FAILURE;
}
```
Or you can get the description of a specific error code:
```c
ctorm_info("BadAddress: %s", ctorm_geterror_from_code(BadAddress));
```
