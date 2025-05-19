# Log Functions

ctorm provides a simple, colored logging system for your general logging usage.

### General logging functions

```c
ctorm_info("some information");
ctorm_warn("you better read this");
ctorm_fail("PANIC!!");
```

### Enable/Disable request logging

By default ctorm will log information about every single request, you can
disable/enabled this with the [app configuration](app.md).
