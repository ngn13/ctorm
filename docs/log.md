# Log Functions 
ctorm provides a simple, colored logging system for your 
general logging usage.

### General logging functions 
```c
info("some information");
warn("there may be something wrong");
error("PANIC!!");
```

### Enable/Disable request logging
By default ctorm will log information about every single request 
on the console, you can disable/enabled this with the [app configuration](app.md).
