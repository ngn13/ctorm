FROM ghcr.io/ngn13/ctorm:latest

WORKDIR /example/echo
COPY static ./static
COPY html   ./html
COPY main.c ./

# examples uses the local headers, replace it with the global ones
RUN gcc -O3 -o /app main.c -lctorm

WORKDIR /
CMD ["/app"]
