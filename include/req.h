#pragma once
#include "http.h"
#include "table.h"

typedef struct req_t {
  method_t method;  // HTTP method (GET, POST, PUT etc.)
  char    *encpath; // url encoded path (does include queries)
  char    *path;    // url decoded path (does not include queries)
  char    *version; // HTTP version number (for example "HTTP/1.1")
  char    *addr;    // TCP connection address

  table_t headers; // HTTP headers
  table_t query;   // HTTP queries (for example "?key=1")

  size_t bodysize; // size of the HTTP body
  char  *body;     // raw body, does NOT have a NULL terminator
} req_t;

typedef table_t body_t; // parsed body is just a table

void req_init(req_t *);          // setup a request
void req_free(req_t *);          // cleanup a request
size_t req_size(req_t *);          // get the request size (used with req_tostr())
void req_tostr(req_t *, char *); // convert the request to string

char *req_method(req_t *);         // get the request method (GET, POST, PUT etc.)
bool req_body(req_t *, char *);           // get the request body, with a NULL terminator (printable)
char *req_query(req_t *, char *);  // get a request URL query
char *req_header(req_t *, char *); // get a request header

body_t *req_body_parse(req_t *);        // parse the URL encoded form body
char   *req_body_get(body_t *, char *); // get a key/value from the body
void    req_body_free(body_t *);        // free the body

void req_add_header(req_t *, char *);       // add a new header to request (internal)
void req_add_header_value(req_t *, char *); // set the last header value (internal)
