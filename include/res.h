#pragma once

#include "connection.h"
#include "encoding.h"
#include "headers.h"
#include "http.h"

typedef struct res_t {
  connection_t *con; // socket connection

  const char    *version; // HTTP version
  unsigned short code;    // HTTP response code

  char    *body;     // HTTP response body
  uint64_t bodysize; // HTTP response body size
  int      bodyfd;   // file descriptor associated with the body

  headers_t headers; // HTTP headers

  bool completed; // set to true if response is transferred
} res_t;

void res_init(res_t *, connection_t *); // setup a response
void res_free(res_t *);                 // cleanup a response

bool res_fmt(res_t *, const char *, ...); // set the response body using a format
bool res_add(res_t *, const char *, ...); // add data to response body
void res_send(res_t *, char *, uint64_t); // set response body data
bool res_sendfile(res_t *, char *);       // send file data in the body data
void res_clear(res_t *);                  // clear the response body
bool res_json(res_t *, cJSON *);          // set the response body to a JSON object

void res_set(res_t *, char *, char *); // set a response header
void res_del(res_t *, char *);         // delete a response header
void res_redirect(res_t *, char *);    // set the "Location" header to redirect

bool res_end(res_t *); // transfers the actual response
