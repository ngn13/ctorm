#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*!

 * @brief HTTP methods

 * This enum stores different types of HTTP request methods as defined in the
 * section "4. Request Methods" of RFC 7231. Only method that is not implemented
 * is the CONNECT method as it's practically useless in a library like ctorm.

*/
typedef enum {
  /*!

   * @brief HTTP GET method

   * This method is explained in section "4.3.1. GET":
   * "The GET method requests transfer of a current selected representation
   * for the target resource.  GET is the primary mechanism of information
   * retrieval and the focus of almost all performance optimizations."

   * Defined in   : HTTP/1.0, HTTP/1.1
   * Request body : not allowed
   * Response body: allowed

   * @note RFC technically allows GET requests to contain a body. However most
   *       of the implementations does not, and they reject any GET request with
   *       a body. This is also the case for ctorm.

  */
  CTORM_HTTP_GET = 1,

  /*!

   * @brief HTTP HEAD method

   * This method is explained in section "4.3.2 HEAD":
   * "The HEAD method is identical to GET except that the server MUST NOT send a
   * message body in the response (i.e., the response terminates at the end of
   * the header section)."

   * Defined in   : HTTP/1.0, HTTP/1.1
   * Request body : not allowed
   * Response body: not allowed

   * @note Similar to the GET request, RFC technically allows HEAD requests to
   *       contain a body. However most implementations do not allow this. This
   *       is also the case for ctorm.

  */
  CTORM_HTTP_HEAD,

  /*!

   * @brief HTTP POST method

   * This method is explained in section "4.3.3. POST":
   * "The POST method requests that the target resource process the
   * representation enclosed in the request according to the resource's own
   * specific semantics"

   * Defined in   : HTTP/1.0, HTTP/1.1
   * Request body : required
   * Response body: allowed

  */
  CTORM_HTTP_POST,

  /*!

   * @brief HTTP PUT method

   * This method is explained in section "4.3.4. PUT":
   * "The PUT method requests that the state of the target resource be created
   * or replaced with the state defined by the representation enclosed in the
   * request message payload"

   * Defined in   : HTTP/1.0 (as an addition), HTTP/1.1
   * Request body : required
   * Response body: allowed

  */
  CTORM_HTTP_PUT,

  /*!

   * @brief HTTP DELETE method

   * This method is explained in section "4.3.5. DELETE":
   * "The DELETE method requests that the origin server remove the association
   * between the target resource and its current functionality."

   * Defined in   : HTTP/1.0 (as an addition), HTTP/1.1
   * Request body : allowed
   * Response body: allowed

  */
  CTORM_HTTP_DELETE,

  /*!

   * @brief HTTP CONNECT method

   * This method is explained in section "4.3.6. CONNECT":
   * "The CONNECT method requests that the recipient establish a tunnel to the
   * destination origin server..."

   * Defined in   : HTTP/1.1
   * Request body : not allowed
   * Response body: not allowed

  */
  CTORM_HTTP_CONNECT,

  /*!

   * @brief HTTP OPTIONS method

   * This method is explained in section "4.3.7. OPTIONS":
   * "The OPTIONS method requests information about the communication options
   * available for the target resource, at either the origin server or an
   * intervening intermediary."

   * Defined in   : HTTP/1.1
   * Request body : allowed
   * Response body: allowed

  */
  CTORM_HTTP_OPTIONS,

  /*!

   * @brief HTTP TRACE method

   * This method is explained in section "4.3.8. TRACE":
   * "The TRACE method requests a remote, application-level loop-back of the
   * request message. The final recipient of the request SHOULD reflect the
   * message received, excluding some fields described below, back to the client
   * as the message body of a 200 (OK) response..."

   * Defined in   : HTTP/1.1
   * Request body : not allowed
   * Response body: required

  */
  CTORM_HTTP_TRACE
} ctorm_http_method_t;

/*!

 * @brief HTTP version numbers

 * This enum contains supported HTTP version numbers

*/
typedef enum {
  CTORM_HTTP_1_0 = 1, /// HTTP/1.0 (RFC 1945)
  CTORM_HTTP_1_1,     /// HTTP/1.1 (RFC 7230, 7231)
} ctorm_http_version_t;

/// HTTP response code type
typedef uint16_t ctorm_http_code_t;

#ifndef CTORM_EXPORT

// describes a single HTTP request method
struct ctorm_http_method_desc {
  const char *name;

  bool allows_req_body;
  bool allows_res_body;

  bool requires_req_body;
  bool requires_res_body;
};

// list of HTTP methods, indexed by ctorm_http_method_t
extern struct ctorm_http_method_desc ctorm_http_methods[];

// static values calculated at compile time
#define CTORM_HTTP_VERSION_LEN 8   // "HTTP/x.x"
#define CTORM_HTTP_METHOD_MAX  7   // "OPTIONS" (or "CONNECT")
#define CTORM_HTTP_CODE_MIN    100 // min response code
#define CTORM_HTTP_CODE_MAX    599 // max response code

// dynamic values calculated at runtime
extern uint64_t ctorm_http_target_max;       // max HTTP request target length
extern uint64_t ctorm_http_header_name_max;  // max HTTP header name length
extern uint64_t ctorm_http_header_value_max; // max HTTP header value length

// initializes & calculates all the dynamic values
void ctorm_http_load();

// macros to check the HTTP response codes
#define ctorm_http_code_is_valid(code)                                         \
  ((code) >= CTORM_HTTP_CODE_MIN && (code) <= CTORM_HTTP_CODE_MAX)
#define ctorm_http_code_is_error(code)                                         \
  ((code) >= 400 && CTORM_HTTP_CODE_MAX >= (code))

// get version/method from the string representation of it
ctorm_http_version_t ctorm_http_version(char *version);
ctorm_http_method_t  ctorm_http_method(char *method);

/*

 * these method functions does not check the method argument, so the caller
 * should make sure it's valid, ideally by obtaining it with ctorm_http_method()

*/
#define ctorm_http_method_name(method) (ctorm_http_methods[(method) - 1].name)

#define ctorm_http_method_allows_req_body(method)                              \
  (ctorm_http_methods[(method) - 1].allows_req_body)
#define ctorm_http_method_allows_res_body(method)                              \
  (ctorm_http_methods[(method) - 1].allows_res_body)

#define ctorm_http_method_needs_req_body(method)                               \
  (ctorm_http_methods[(method) - 1].requires_req_body)
#define ctorm_http_method_needs_res_body(method)                               \
  (ctorm_http_methods[(method) - 1].requires_res_body)

bool ctorm_http_is_valid_header_name(char *name, uint64_t size);
bool ctorm_http_is_valid_header_value(char *value, uint64_t size);

#endif
