```
        __                     
  _____/ /__________  ____ ___ 
 / ___/ __/ ___/ __ \/ __ `__ \
/ /__/ /_/ /  / /_/ / / / / / /
\___/\__/_/   \____/_/ /_/ /_/ 1.3
                               
```

# ctorm | simple web framework for C
![](https://img.shields.io/github/actions/workflow/status/ngn13/ctorm/docker.yml)
![](https://img.shields.io/github/v/tag/ngn13/ctorm?label=version)
![](https://img.shields.io/github/license/ngn13/ctorm)

ctorm is a libevent based, multi-threaded HTTP server for `HTTP/1.1` and `HTTP/1.0`.
It has a (fairly) easy API for general web server applications. 

### Important!!!
This software is pretty much in alpha state. I don't suggest you use ctorm on
production, however it can be used to build simple web applications just for fun.

I do plan to continue the development of this project, so please consider contributing
if you are interested.

### Features
- Wildcard routes
- Middleware support
- Form body parsing
- URL queries (parameters)
- JSON support with [cJSON](https://github.com/DaveGamble/cJSON)
- Handling 404 (all) routes
- Sending files and static file serving

### Benchmarking
Benchmark results for hello world applications (see [benchmark](benchmark/)):

| Framework        | Version       | Time per request | 
| ---------------- | ------------- | ---------------- |
| crow (C++)       | v1.2.0        | ~4 ms            |
| fiber (Go)       | v3.0.0-beta.1 | ~4 ms            |
| **ctorm (C)**    | **1.3**       | **~5 ms**        |
| tide (Rust)      | 0.16.0        | ~12 ms           |
| express (NodeJS) | 4.19.2        | ~21 ms           |

### Installation
You will need the following software in order to build and install ctorm:
- GCC and other general build tools (`build-essential`)
- libevent and it's headers (`libevent`, `libevent-dev`)
- cJSON and it's headers (`cjson`, `libcjson-dev`)
- tar (to extract the release archive)

First [download the latest release](https://github.com/ngn13/ctorm/tags) archive,
**do not compile from the latest commit unless you are doing development**:
```bash
wget https://github.com/ngn13/ctorm/archive/refs/tags/1.3.tar.gz
tar xf 1.3.tar.gz && cd ctorm-1.3
```

Then use the `make` command to build and install:
```bash
make && sudo make install
```

### Getting started
#### Hello world application 
```c
#include <ctorm/all.h>

void hello_world(req_t *req, res_t *res) {
  // send the "Hello world!" message
  RES_SEND("Hello world!");
}

int main() {
  // create the app with default configuration
  app_t *app = app_new(NULL);

  // setup the routes
  GET(app, "/", hello_world);

  // run the app
  if (!app_run(app, "0.0.0.0:8080"))
    error("app failed: %s\n", app_geterror());

  // clean up
  app_free(app);
  return 0;
}
```

#### Other functions 
- [App](docs/app.md)
- [Logging](docs/log.md)
- [Request](docs/req.md)
- [Response](docs/res.md)

#### Example applications
Repository also contains few example applications in the `example` folder, you can
build these by running `make example`.

#### Deploying your application
You can use the docker image (built with actions) to easily deploy your application, here is 
an example:
```Dockerfile
FROM ghcr.io/ngn13/ctorm:latest 

WORKDIR /app

# copy over all the sources
COPY src       ./src
COPY Makefile  ./
COPY main.c    ./

# run the make script
RUN make 

# set the startup command
CMD ["/app/server"]
```

### Development 
For development, you can compile the library with debug mode:
```bash
make DEBUG=1
```
then you can use the example applications for testing:
```bash
make example
LD_LIBRARY_PATH=./dist ./dist/example_hello
```
