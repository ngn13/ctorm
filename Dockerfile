FROM ubuntu as build

RUN apt update
RUN apt install -y gcc make libcjson-dev dumb-init

WORKDIR       /pkg
COPY src      ./src
COPY include  ./inc
COPY Makefile ./

RUN make
RUN make install

WORKDIR /
RUN rm -fr /pkg

ENTRYPOINT ["/usr/bin/dumb-init", "--"]
