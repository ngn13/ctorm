# ctorm | simple web framework for C
![](https://img.shields.io/github/actions/workflow/status/ngn13/ctorm/docker.yml)

ctorm is a libevent based, multi-threaded HTTP server for `HTTP/1.1` and `HTTP/1.0`.
It has a (fairly) easy API for general web server applications. 

### Important!!!
This software is pretty much in alpha state. I don't you should be using ctorm on
production, however it can be used to build simple web applications just for fun.

I do plan to continue the development of this project, so please consider contributing
if you are interested.

### Features
- URL queries (parameters)
- Form body parsing
- Simple dynamic 'rendering'
- Sending files
- Handling 404 (all) routes
- Regex based routing

### Benchmarking
Benchmark results for hello world applications (see [benchmark](benchmark/)):

| Framework        | Time per request | 
| ---------------- | ---------------- |
| Crow (C++)       | 3.983 ms         |
| Fiber (Go)       | 4.818 ms         |
| **Ctorm (C)**    | **5.071 ms**     |
| Tide (Rust)      | 10.584 ms        |
| Express (NodeJS) | 24.873 ms        |

### Installation
You will need the following software in order to build and install ctorm:
- GCC and other general build tools (`build-essential`)
- libevent and it's headers (`libevent`, `libevent-dev`)
- git (or you can just download the repository as an archive)

First clone the repository:
```bash
git clone https://github.com/ngn13/ctorm.git
cd ctorm
```

Then use the `make` command for the build and install:
```bash
make && sudo make install
```

### Getting started
#### Hello world application 
```c
#include <ctorm/ctorm.h>
#include <stdlib.h>
#include <stdio.h>

// this function handles GET request that go to '/'
void hello_world(req_t* req, res_t* res){
  // just send the string 'hello world!'
  res_send(res, "hello world!");
}

// main function (entrypoint)
int main(){
  // init the application
  app_init();

  // setup a GET route for '/'
  GET("/", hello_world);
  
  // start the application on port 8080 
  app_run("127.0.0.1:8080");
}
```

#### Other functions 
- [App](docs/app.md)
- [Logging](docs/log.md)
- [Request](docs/req.md)
- [Response](docs/res.md)
- [Rendering](docs/render.md)

#### Example application 
Repository also contains an example application in the `example` folder, you can build 
this app with `make test`.

#### Deploying your application
You can use the docker image (built with actions) to easily deploy your application, here is 
an example:
```Dockerfile
FROM ghcr.io/ngn13/ctorm:latest 

WORKDIR /app

# copy over all the sources
COPY template  ./template
COPY src       ./src
COPY Makefile  ./
COPY main.c    ./

# run the make script
RUN make 

# set the startup command
CMD ["/app/server"]
```

### Development 
For development purposes, you can use the example application, and compile the library with debug mode:
```bash
make DEBUG=1 && make test
```

---
<img src="https://files.ngn.tf/gpl3.png" width="200px">
