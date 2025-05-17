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

> [!WARNING] This software is pretty much in alpha state. I don't suggest you
> use ctorm on production, however it can be used to build simple web
> applications just for fun.

### Features

- Express-like HTTP routes
- Multi-thread support (multiple servers in one program)
- URL queries and parameters
- Form encoded body parsing
- JSON support with [cJSON](https://github.com/DaveGamble/cJSON)
- Wildcard routes
- Default (all) route
- Sending files and static file serving

### Installation

You will need the following software in order to build and install ctorm:

- GNU tar to extract the release archive (`tar`)
- GCC and other general build tools (`build-essential`)
- If you want to build the man pages, [`doxygen`](https://www.doxygen.org/)
- If you want JSON support, cJSON and it's headers (`cjson`, `libcjson-dev`)

First
[download the latest release archive](https://github.com/ngn13/ctorm/tags), **do
not compile from the latest commit or a branch unless you are doing
development**:

```bash
wget https://github.com/ngn13/ctorm/archive/refs/tags/1.5.tar.gz
tar xf 1.5.tar.gz && cd ctorm-1.5
```

Then use the `make` command to compile the library:

```bash
make
```

**If you don't have cJSON installed**, you need to run this command with
`CTORM_JSON_SUPPORT=0` option to disable JSON support:

```bash
make CTORM_JSON_SUPPORT=0
```

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

### Getting started

#### Hello world application

```c
#include <ctorm/ctorm.h>

void GET_index(ctorm_req_t *req, ctorm_res_t *res) {
  // send the "Hello world!" message
  RES_SEND("Hello world!");
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

#### Other functions

Here are some nicely formatted markdown documents that explain all the functions
you will most likely gonna use:

- [App](docs/app.md)
- [Error](docs/error.md)
- [Logging](docs/log.md)
- [Request](docs/req.md)
- [Response](docs/res.md)

You can also checkout the man pages if you built and installed during the
[installation](#installation).

#### Example applications

Repository also contains few example applications in the `example` directory.
You can build these by running:

```bash
make example
```

#### Deploying your application

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

### Development

For development, you can compile the library with the `CTORM_DEBUG=1` option to
enable debug messages:

```bash
make CTORM_DEBUG=1
```

Then you can use the example applications and the test scripts in the `scripts`
directory for testing:

```bash
make test
```
