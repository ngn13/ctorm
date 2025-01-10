#pragma once

#include "connection.h"
#include "encoding.h"
#include "headers.h"
#include "http.h"

typedef struct ctorm_req {
  connection_t *con; // socket connection

  method_t    method; // HTTP method (GET, POST, PUT etc.)
  bool        cancel;
  char       *encpath; // url encoded path (does include queries)
  char       *path;    // url decoded path (does not include queries)
  const char *version; // HTTP version number (for example "HTTP/1.1")

  headers_t  headers;          // HTTP headers
  bool       received_headers; // did we receive all the HTTP headers
  enc_url_t *queries;          // HTTP queries (for example "?key=1")
  pair_t    *params;           // HTTP path params (for example "/blog/:slug")
  int64_t    bodysize;         // size of the HTTP body
} ctorm_req_t;

#ifndef CTORM_EXPORT

void ctorm_req_init(ctorm_req_t *, connection_t *); // setup a request
void ctorm_req_free(ctorm_req_t *);                 // cleanup a request
bool ctorm_req_start(ctorm_req_t *);                // receive the (at least the first part) of the HTTP request
void ctorm_req_end(ctorm_req_t *);                  // completely receive the HTTP request

#endif

char *ctorm_req_method(ctorm_req_t *);        // get the request method (GET, POST, PUT etc.)
char *ctorm_req_query(ctorm_req_t *, char *); // get a request URL query
char *ctorm_req_get(ctorm_req_t *, char *);   // get a request header

uint64_t ctorm_req_body(ctorm_req_t *, char *, uint64_t); // copy given length of body to the buffer
uint64_t ctorm_req_body_size(ctorm_req_t *);              // get the body size

char *ctorm_req_ip(ctorm_req_t *, char *); // get the requester IPv4/IPv6 address as string
#define ctorm_req_addr(r) ((r)->con->addr) // get the requester address as sockaddr

enc_url_t *ctorm_req_form(ctorm_req_t *); // parse URL encoded form body
cJSON     *ctorm_req_json(ctorm_req_t *); // parse JSON encoded form body
