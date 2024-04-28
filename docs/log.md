# Log Functions 
Ctorm provides a simple, colored logging system for your 
general logging usage.

### General logging functions 
```c
info("some information");
warn("there may be something wrong");
error("PANIC!!");
```

### Enable/Disable request logging
By default ctorm will log information about every single request 
on the console, you can disable/enabled this:
```c
log_set(true);  // enable request logging
log_set(false); // disable request logging
```
