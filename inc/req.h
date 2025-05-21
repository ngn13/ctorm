/*!

 * @file
 * @brief Header file that defines HTTP request structure and functions

*/
#pragma once

#include "encoding.h"
#include "headers.h"

#include "conn.h"
#include "http.h"
#include "pair.h"

/*!

 * @brief HTTP request structure

 * Stores information about HTTP request, such as the request method, path,
 * version, headers etc.

*/
typedef struct {
  ctorm_conn_t *conn; /// client connection

  ctorm_pair_t     *locals; /// local variables to pass along with the request
  bool              cancel; /// is the request cancelled?
  ctorm_http_code_t code;   /// default HTTP response code for this request

  ctorm_http_method_t  method;  /// HTTP method (GET, POST, PUT etc.)
  ctorm_http_version_t version; /// HTTP version number (HTTP/1.1 etc.)

  char *target; /// HTTP request target (see "5.3. Request Target")
  char *host;   /// target host
  char *path;   /// target path (URL decoded, does not include queries)

  ctorm_query_t *queries; /// HTTP queries (for example "?key=1")
  ctorm_pair_t  *params;  /// HTTP path parameters (for example "/blog/:slug")

  ctorm_headers_t headers;   /// HTTP headers
  int64_t         body_size; /// remaining size of HTTP request body
  cJSON          *body_json; /// JSON decoded body
  ctorm_query_t  *body_form; /// URL form decoded body
} ctorm_req_t;

#ifndef CTORM_EXPORT

void ctorm_req_init(ctorm_req_t *req, ctorm_conn_t *conn); // init HTTP request
void ctorm_req_free(ctorm_req_t *req); // free a HTTP request
bool ctorm_req_recv(ctorm_req_t *req); // receive the HTTP request

#endif

/*!

 * Get the request method

 * @param[in] req: HTTP request
 * @return    HTTP request method as string ("GET", "POST" etc.)

*/
const char *ctorm_req_method(ctorm_req_t *req);

/*!

 * Get the value associated with a provided URI query name

 * @param[in] req: HTTP request
 * @param[in] name: Query name
 * @return    Query value

*/
char *ctorm_req_query(ctorm_req_t *req, char *name);

/*!

 * Get the value for a route parameter

 * @param[in] req: HTTP request
 * @param[in] name: Parameter name
 * @return    Parameter value

*/
char *ctorm_req_param(ctorm_req_t *req, char *name);

/*!

 * Get a local variable from the request. If no value is provided, the function
 * returns the value associated with the provided name. Also see @ref
 * ctorm_app_local

 * @param[in] req: HTTP request
 * @param[in] name: Local variable name
 * @param[in] value: Local variable value
 * @return    Local variable value

*/
void *ctorm_req_local(ctorm_req_t *req, char *name, char *value);

/*!

 * Get HTTP request header

 * @param[in] req: HTTP request
 * @param[in] header: Header name
 * @return    Header value

*/
char *ctorm_req_get(ctorm_req_t *req, char *header);

/*!

 * Copy a given amount of bytes from request body to the buffer

 * @param[in]  req: HTTP request
 * @param[out] buf: Destination buffer
 * @param[in]  size: Amount of bytes to copy
 * @return     Amount of bytes that has been copied (smaller than or equal to
 *             the size argument

*/
int64_t ctorm_req_body(ctorm_req_t *req, char *buf, int64_t size);

/*!

 * Copy the sender's IPv4 or IPv6 address to the provided buffer as string.
 * Buffer should at least be INET6_ADDRSTRLEN+1 bytes long. If no buffer is
 * provided, function will allocate a buffer from the heap, and return it's
 * address. You should free this buffer when you are done with it.

 * @param[in]  req: HTTP request
 * @param[out] buf: Destination buffer
 * @return     Pointer to the start of the IP string

*/
#define ctorm_req_ip(req, buf) ctorm_conn_ip((req)->conn, buf)

/*!

 * @brief     Get socket address (struct sockaddr) of the client
 * @param[in] req: HTTP request
 * @return    Socket address (struct sockaddr)

*/
#define ctorm_req_addr(req) ((req)->conn->addr)

/*!

 * Parse URL encoded form body. Only works if the body is URL encoded and if
 * it's using the content type application/x-www-form-urlencoded. After
 * obtaining a @ref ctorm_query_t pointer by calling this function, you can read
 * different key-value pairs using @ref ctorm_query_get. However do NOT free
 * this structure using @ref ctorm_query_free, this is done internally when the
 * processing of the request is complete

 * @param[in] req: HTTP request
 * @return    Form data

*/
ctorm_query_t *ctorm_req_form(ctorm_req_t *req);

/*!

 * Parse JSON encoded form body. Only works if the body is JSON encoded and if
 * it's using the content type application/json. This function will provide you
 * a cJSON structure that you can use with the cJSON library. However do NOT
 * free this structure using the cJSON functions, this is done internally when
 * the processing of the request is complete

 * @param[in] req: HTTP request
 * @return    JSON decoded data

*/
cJSON *ctorm_req_json(ctorm_req_t *req);

/*!

 * Check if the HTTP request will persist or not. This is explained in the
 * "6.3. Persistence" section of RFC 7230. Please note that a client may still
 * unexpectedly or maliciously close the connection even though the connection
 * is supposed to be persistent

 * @param[in] req: HTTP request
 * @return    True if the request wants a persistent HTTP connection

*/
bool ctorm_req_persist(ctorm_req_t *req);
