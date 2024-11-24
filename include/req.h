#pragma once

#include <arpa/inet.h>
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#else
typedef void cJSON;
#endif

#include "http.h"
#include "table.h"
#include "form.h"
#include "headers.h"

typedef struct req_t {
  method_t method; // HTTP method (GET, POST, PUT etc.)
  bool     cancel;
  char    *encpath;                // url encoded path (does include queries)
  char    *path;                   // url decoded path (does not include queries)
  char    *version;                // HTTP version number (for example "HTTP/1.1")
  char     addr[INET6_ADDRSTRLEN]; // TCP connection address

  headers_t headers; // HTTP headers
  table_t   query;   // HTTP queries (for example "?key=1")

  size_t bodysize; // size of the HTTP body
  char  *body;     // raw body, does NOT have a NULL terminator
} req_t;

void   req_init(req_t *);          // setup a request
void   req_free(req_t *);          // cleanup a request
size_t req_size(req_t *);          // get the request size (used with req_tostr())
void   req_tostr(req_t *, char *); // convert the request to string

char  *req_method(req_t *);        // get the request method (GET, POST, PUT etc.)
bool   req_body(req_t *, char *);  // get the request body, with a NULL terminator (printable)
size_t req_body_size(req_t *);     // get the body size that will be returned with req_body()
char  *req_query(req_t *, char *); // get a request URL query

void  req_set(req_t *req, char *name, char *value, bool dup); // set a request header
char *req_get(req_t *, char *);                               // get a request header

bool req_form_parse(req_t *, form_t *); // parse the URL encoded form body
void req_form_free(form_t *);           // free the parsed form body

cJSON *req_json_parse(req_t *); // parse the JSON formatted body
void   req_json_free(cJSON *);  // free the parsed JSON body
