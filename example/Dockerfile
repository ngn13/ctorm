FROM ghcr.io/ngn13/ctorm:latest

WORKDIR /example
COPY template  ./template
COPY static    ./static
COPY 404.html  ./
COPY main.c    ./

# example code uses the local headers,
# replace it with the global ones
RUN sed -i 's/#include "..\/include\/ctorm.h"/#include <ctorm\/ctorm.h>/' main.c  
RUN gcc -o /app main.c -lctorm

WORKDIR /
CMD ["/app"]
