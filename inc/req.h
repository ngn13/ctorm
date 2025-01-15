/*!

 * @file
 * @brief Header file that defines HTTP request structure and functions

*/
#pragma once

#include "connection.h"
#include "encoding.h"
#include "headers.h"
#include "http.h"
#include "pair.h"

/*!

 * @brief HTTP request structure

 * Stores information about HTTP request, such as the request method,
 * path, version, headers etc.

*/
typedef struct {
  connection_t *con; /// socket connection

  method_t    method; /// HTTP method (GET, POST, PUT etc.)
  bool        cancel;
  char       *encpath; /// url encoded path (does include queries)
  char       *path;    /// url decoded path (does not include queries)
  const char *version; /// HTTP version number (for example "HTTP/1.1")

  ctorm_headers_t headers;          /// HTTP headers
  bool            received_headers; /// did we receive all the HTTP headers
  ctorm_url_t    *queries;          /// HTTP queries (for example "?key=1")
  ctorm_pair_t   *params;           /// HTTP path params (for example "/blog/:slug")
  ctorm_pair_t   *locals;           /// Local variables to pass along with the request
  int64_t         bodysize;         /// size of the HTTP body
} ctorm_req_t;

#ifndef CTORM_EXPORT

#define ctorm_req_is_valid(req)                                                                                        \
  (NULL != (req)->version && NULL != (req)->encpath && NULL != (req)->path) // check if the request is valid
void ctorm_req_init(ctorm_req_t *req, connection_t *con);                   // setup a request
void ctorm_req_free(ctorm_req_t *req);                                      // cleanup a request
bool ctorm_req_start(ctorm_req_t *req); // receive the (at least the first part) of the HTTP request
void ctorm_req_end(ctorm_req_t *req);   // completely receive the HTTP request

#endif

const char *ctorm_req_method(ctorm_req_t *req);                 // get the request method (GET, POST, PUT etc.)
char       *ctorm_req_query(ctorm_req_t *req, char *name);      // get a request URL query
char       *ctorm_req_param(ctorm_req_t *req, char *name);      // get a request URL param
void       *ctorm_req_local(ctorm_req_t *req, char *name, ...); // get or set a local by name
char       *ctorm_req_get(ctorm_req_t *req, char *header);      // get a request header

uint64_t ctorm_req_body(
    ctorm_req_t *req, char *buf, uint64_t size); // copy given amount of bytes from body to the buffer
uint64_t ctorm_req_body_size(ctorm_req_t *req);  // get the body size

char *ctorm_req_ip(ctorm_req_t *req, char *);  // get the requester IPv4/IPv6 address as string
#define ctorm_req_addr(req) ((req)->con->addr) // get the requester address as sockaddr

ctorm_url_t *ctorm_req_form(ctorm_req_t *req); // parse URL encoded form body
cJSON       *ctorm_req_json(ctorm_req_t *req); // parse JSON encoded form body
