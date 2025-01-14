/*!

 * @file
 * @brief Header file that defines HTTP response structure and functions

*/
#pragma once

#include "connection.h"
#include "encoding.h"
#include "headers.h"

/*!

 * @brief HTTP response structure

 * Stores information about HTTP response, such as the response code,
 * body, version, headers etc.

*/
typedef struct {
  connection_t *con; /// socket connection

  ctorm_headers_t headers; /// HTTP headers
  const char     *version; /// HTTP version
  unsigned short  code;    /// HTTP response code

  char    *body;     /// HTTP response body
  uint64_t bodysize; /// HTTP response body size
  int      bodyfd;   /// file descriptor associated with the body

  bool completed; /// set to true if response is transferred
} ctorm_res_t;

#ifndef CTORM_EXPORT

void ctorm_res_init(ctorm_res_t *, connection_t *); // setup a response
void ctorm_res_free(ctorm_res_t *);                 // cleanup a response
bool ctorm_res_end(ctorm_res_t *);                  // transfers the actual response

#endif

bool ctorm_res_fmt(ctorm_res_t *, const char *, ...); // set the response body using a format
bool ctorm_res_add(ctorm_res_t *, const char *, ...); // add data to response body
void ctorm_res_send(ctorm_res_t *, char *, uint64_t); // set response body data
bool ctorm_res_sendfile(ctorm_res_t *, char *);       // send file data in the body data
void ctorm_res_clear(ctorm_res_t *);                  // clear the response body
bool ctorm_res_json(ctorm_res_t *, cJSON *);          // set the response body to a JSON object

void ctorm_res_set(ctorm_res_t *, char *, char *); // set a response header
void ctorm_res_del(ctorm_res_t *, char *);         // delete a response header
void ctorm_res_redirect(ctorm_res_t *, char *);    // set the "Location" header to redirect
