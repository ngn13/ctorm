FROM ubuntu as build

RUN apt update
RUN apt install -y gcc make libcjson-dev libevent-dev dumb-init

WORKDIR       /pkg
COPY src      ./src
COPY include  ./include
COPY Makefile ./

RUN make
RUN make install

WORKDIR /
RUN rm -r /pkg
ENTRYPOINT ["/usr/bin/dumb-init", "--"]
