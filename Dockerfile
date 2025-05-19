FROM debian

RUN apt update && \
    apt install --no-install-recommends -y \
      build-essential libcjson-dev dumb-init && \
    rm -rf /var/lib/apt/lists/*

WORKDIR       /pkg
COPY src      ./src
COPY inc      ./inc
COPY Makefile ./

RUN make
RUN make install

WORKDIR /
RUN rm -fr /pkg

ENTRYPOINT ["/usr/bin/dumb-init", "--"]
