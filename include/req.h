#pragma once

#include "connection.h"
#include "encoding.h"
#include "headers.h"
#include "headers.h"

#include "http.h"
#include "form.h"

typedef struct req_t {
  connection_t *con; // socket connection

  method_t    method; // HTTP method (GET, POST, PUT etc.)
  bool        cancel;
  char       *encpath; // url encoded path (does include queries)
  char       *path;    // url decoded path (does not include queries)
  const char *version; // HTTP version number (for example "HTTP/1.1")

  headers_t headers;          // HTTP headers
  bool      received_headers; // did we receive all the HTTP headers
  urlenc_t *queries;          // HTTP queries (for example "?key=1")
  int64_t   bodysize;         // size of the HTTP body
} req_t;

void req_init(req_t *, connection_t *); // setup a request
void req_free(req_t *);                 // cleanup a request

bool req_start(req_t *); // receive the (at least the first part) of the HTTP request
void req_end(req_t *);   // completely receive the HTTP request

char *req_method(req_t *);        // get the request method (GET, POST, PUT etc.)
char *req_query(req_t *, char *); // get a request URL query
char *req_get(req_t *, char *);   // get a request header

uint64_t req_body(req_t *, char *, uint64_t); // copy given length of body to the buffer
uint64_t req_body_size(req_t *);              // get the body size

char *req_ip(req_t *, char *);       // get the requester IPv4/IPv6 address as string
#define req_addr(r) ((r)->con->addr) // get the requester address as sockaddr

urlenc_t *req_form(req_t *); // parse URL encoded form body
cJSON    *req_json(req_t *); // parse JSON encoded form body
