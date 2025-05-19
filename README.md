```
        __
  _____/ /_____  _________ ___
 / ___/ __/ __ \/ ___/ __ `__ \
/ /__/ /_/ /_/ / /  / / / / / /
\___/\__/\____/_/  /_/ /_/ /_/ 1.8

```

# ctorm | simple web framework for C

![](https://img.shields.io/github/actions/workflow/status/ngn13/ctorm/docker.yml)
![](https://img.shields.io/github/actions/workflow/status/ngn13/ctorm/test.yml?label=tests)
![](https://img.shields.io/github/v/tag/ngn13/ctorm?label=version)
![](https://img.shields.io/github/license/ngn13/ctorm)

ctorm is a multi-threaded, simple web server microframework for `HTTP/1.1`,
inspired by [Express](https://expressjs.com/),
[Fiber](https://github.com/gofiber/fiber) and similar HTTP frameworks.

This library is pretty much in alpha state. I don't suggest you use ctorm on
production, however it can be used to build simple web servers and applications
just for fun.

## Features

- Express-like HTTP routes
- Multi-thread support (multiple servers in one program)
- URL queries and parameters
- Form encoded body parsing
- JSON support with [cJSON](https://github.com/DaveGamble/cJSON)
- Wildcard routes
- Default (all) route
- Sending files and static file serving

## Supported systems

Here is a list of all the supported platforms and architectures:

- **Supported**: Should work, but not thoroughly tested. Report any issues
- **Tested**: Should work, well tested. Report any issues
- **Planned**: Support planned for the future. May or may not work, not tested
  at all. Do not report issues, feel free help me add support.

| **Platform/Arch** | **amd64** | **i386**  |
| ----------------- | --------- | --------- |
| **GNU/Linux**     | Tested    | Supported |
| **FreeBSD**       | Supported | Tested    |
| **OpenBSD**       | Planned   | Planned   |

## Installation

You will need the following software in order to build and install ctorm:

- GNU tar to extract the release archive (`tar`)
- GCC, GNU make other build tools (`build-essential`)
- If you want to build the man pages, [`doxygen`](https://www.doxygen.org/)
- If you want JSON support, cJSON and it's headers (`cjson`, `libcjson-dev`)

First
[download the latest release archive](https://github.com/ngn13/ctorm/tags), **do
not compile from the latest commit or a branch unless you are doing
development**:

```bash
wget https://github.com/ngn13/ctorm/archive/refs/tags/1.8.tar.gz
tar xf 1.8.tar.gz && cd ctorm-1.8
```

Then use the `make` command to compile the library:

```bash
make
```

Here are all the compile options you can pass to `make`:

| Option               | Description                    | Default        |
| -------------------- | ------------------------------ | -------------- |
| `CTORM_JSON_SUPPORT` | Enable JSON support with cJSON | enabled (`1`)  |
| `CTORM_DEBUG`        | Enable debug logging           | disabled (`0`) |

**If you installed `doxygen`, and you want to build the man pages** run `make`
with the `docs` command:

```bash
make docs
```

To install the library (and if you've built it, the documentation) run `make`
with the `install` command **as root**:

```bash
make install
```

## Getting started

### Hello world application

```c
#include <ctorm/ctorm.h>

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  // send the "Hello world!" message
  RES_BODY("Hello world!");
}

int main() {
  // create the app with default configuration
  ctorm_app_t *app = ctorm_app_new(NULL);

  // setup the routes
  GET(app, "/", GET_index);

  // run the app
  if (!ctorm_app_run(app, "0.0.0.0:8080"))
    ctorm_fail("failed to start the application: %s", ctorm_geterror());

  // clean up
  ctorm_app_free(app);
  return 0;
}
```

### Documentation

Here are some nicely formatted markdown documents that explain all the functions
and the macros you will most likely gonna use:

- [App](docs/app.md)
- [Error](docs/error.md)
- [Logging](docs/log.md)
- [Request](docs/req.md)
- [Response](docs/res.md)

If you built and installed them during the [installation](#installation). You
can also checkout the man pages for different functions, types and macros.

### Example applications

Repository also contains few example applications in the
[`example` directory](example). You can build these by running:

```bash
make example
```

### Deploying your application

You can use the docker image (built by github actions) to easily deploy your
application, here is an example:

```Dockerfile
FROM ghcr.io/ngn13/ctorm:latest

WORKDIR /app

# copy over all the sources
COPY main.c    ./
COPY Makefile  ./

# run the make script
RUN make

# set the startup command
CMD ["/app/server"]
```

## Development

For development, you can compile the library with debug logging (see the
`CTORM_DEBUG` option). Then you can use the example applications and the test
scripts in the `scripts` directory for testing the library:

```bash
make test
```
