FROM ghcr.io/ngn13/ctorm:latest

WORKDIR /example
COPY . ./

RUN mkdir /dist
RUN make                    \
  DONT_CHECK_LIBCTORM=1     \
  INCDIR=/usr/include/ctorm \
  DISTDIR=/dist

WORKDIR /
CMD ["/example/init.sh"]
