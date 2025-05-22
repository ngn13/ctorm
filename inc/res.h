/*!

 * @file
 * @brief Header file that defines HTTP response structure and functions

*/
#pragma once

#include "encoding.h"
#include "headers.h"

#include "conn.h"
#include "http.h"

/*!

 * @brief HTTP response structure

 * Stores information about HTTP response, such as the response code,
 * body, version, headers etc.

*/
typedef struct {
  ctorm_conn_t *conn; /// client connection

  ctorm_http_code_t    code;    /// HTTP response code
  ctorm_http_version_t version; /// HTTP version
  ctorm_headers_t      headers; /// HTTP headers

  char    *body;      /// HTTP response body
  uint32_t body_size; /// HTTP response body size
  int      body_fd;   /// file descriptor associated with the body
} ctorm_res_t;

#ifndef CTORM_EXPORT

void ctorm_res_init(ctorm_res_t *res, ctorm_conn_t *conn); // init HTTP response
void ctorm_res_free(ctorm_res_t *res); // free a HTTP response
bool ctorm_res_send(ctorm_res_t *res); // send the HTTP response

#endif

/*!

 * Set HTTP response code

 * @param[in] code: HTTP response code
 * @return    Returns true if everything goes well

*/
bool ctorm_res_code(ctorm_res_t *res, uint16_t code);

/*!

 * Set the response body using a formatted string. Please note that this
 * completely resets the body, any previous data will be erased

 * @param[in] res: HTTP response
 * @param[in] fmt: Format buffer
 * @return    Returns size of the data (after formatting)

*/
int ctorm_res_fmt(ctorm_res_t *res, const char *fmt, ...);

/*!

 * Does the same thing with @ref ctorm_res_fmt, however instead of clearing the
 * response body, it appends (adds) data to it

 * @param[in] res: HTTP response
 * @param[in] fmt: Format buffer
 * @return    Returns size of the data (after formatting)

*/
int ctorm_res_add(ctorm_res_t *res, const char *fmt, ...);

/*!

 * Copy specified amount of bytes from provided data buffer to the response
 * body. Please note that this reset the response body, erasing any data that
 * has been copied previously.

 * @param[in] res: HTTP response
 * @param[in] data: Data buffer
 * @param[in] size: Amount of bytes to copy. If 0, the function reads the data
 *            buffer until it reaches a NULL termination
 * @return    Returns the amount of copied bytes

*/
uint32_t ctorm_res_body(ctorm_res_t *res, char *data, uint32_t size);

/*!

 * Specify a file to send in the response body. Similar to @ref ctorm_res_body,
 * this clears the body, and erases any previous data that has been copied to
 * it. The function also attempts to detect the content type of the file based
 * on it's extension, and sets the Content-Type header accordingly. If you would
 * like to specify the content type manually, you should do so after calling
 * this function, not before calling it.

 * @param[in] res: HTTP response
 * @param[in] path: Path to the target file
 * @return    Returns true if everything goes well

*/
bool ctorm_res_file(ctorm_res_t *res, char *path);

/*!

 * Clear the response body. This function will erase any data that has been
 * previously copied to the body

 * @param[in] res: HTTP response

*/
void ctorm_res_clear(ctorm_res_t *res);

/*!

 * JSON decodes the provided JSON object, and copies the decoded data to the
 * response body. Please note that this function will erase any previous data
 * has been copied to the response body.

 * @param[in] res: HTTP response
 * @param[in] json: cJSON object, representing the JSON encoded data
 * @return    Returns true if everything goes well

*/
bool ctorm_res_json(ctorm_res_t *res, cJSON *json);

/*!

 * Set a HTTP response header

 * @param[in] res: HTTP response
 * @param[in] name: HTTP header name
 * @param[in] value: HTTP header value

*/
void ctorm_res_set(ctorm_res_t *res, char *name, char *value);

/*!

 * Delete a HTTP response header

 * @param[in] res: HTTP response
 * @param[in] name: HTTP header name

*/
void ctorm_res_del(ctorm_res_t *res, char *name);

/*!

 * Redirect request to a different location. This is done by setting the
 * "Location" header to the provided URI reference and setting the response
 * code to 301 (Moved Permanently)

 * @param[in] res: HTTP response
 * @param[in] uri: URI to redirect

*/
void ctorm_res_redirect(ctorm_res_t *res, char *uri);
