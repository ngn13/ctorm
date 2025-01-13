FROM ghcr.io/ngn13/ctorm:latest

WORKDIR /example/middleware
COPY main.c ./

# examples uses the local headers, replace it with the global ones
RUN gcc -O3 -o /app main.c -lcjson -lctorm

WORKDIR /
CMD ["/app"]
